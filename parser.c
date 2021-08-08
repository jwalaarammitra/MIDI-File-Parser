/* Jwala Aram Mitra, parser.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#include "parser.h"

#include <assert.h>

#include <malloc.h>

#include <stdio.h>

#include <string.h>

#include <unistd.h>

#define HEADER_SIZE_ASSERT (6)
#define BIT_SHIFT_7 (7)
#define BIT_SHIFT_8 (8)
#define BIT_SHIFT_16 (16)
#define BIT_SHIFT_24 (24)
#define VALUE_SHIFT (0x7F)
#define BYTE_SHIFT (0x80)
#define DIVISION_SHIFT (0xFF)
#define HEADER_DIVISION_OPERATOR (0x8000)

uint8_t g_status = 0;

/*
 * This functions takes in the path of a MIDI file and returns the parsed
 * representation of that song.
 */

song_data_t *parse_file(const char *midi) {
  assert(midi != NULL);
  FILE *midi_pointer = fopen(midi, "r");
  assert(midi_pointer != NULL);

  song_data_t *song = NULL;
  song = malloc(sizeof(song_data_t));
  assert(song != NULL);
  song->path = NULL;

  song->track_list = NULL;
  parse_header(midi_pointer, song);

  for (int i = 0; i < song->num_tracks; i++) {
    parse_track(midi_pointer, song);
  }

  int file_end = getc(midi_pointer);
  assert(file_end == EOF);

  fclose(midi_pointer);
  midi_pointer = NULL;

  return song;
} /* parse_file() */

/*
 * This function reads a MIDI header chunk from the given file pointer and
 * update the given song_data_t pointer with the format, number of tracks,
 * and division information.
 */

void parse_header(FILE *midi_pointer, song_data_t *song) {

  char header_chunk_type[4] = "";
  char expected_header_chunk_type[4] = "MThd";

  for (int i = 0; i < sizeof(header_chunk_type); i++) {
    assert((fread(&header_chunk_type[i], sizeof(char), 1, midi_pointer)) == 1);
    assert(header_chunk_type[i] == expected_header_chunk_type[i]);
  }

  uint8_t header_length[4];

  for (int i = 0; i < sizeof(header_length); i++) {
    assert((fread(&header_length[i], sizeof(uint8_t), 1, midi_pointer)) == 1);
  }

  assert(header_length[0] == (uint8_t) 0);
  assert(header_length[1] == (uint8_t) 0);
  assert(header_length[2] == (uint8_t) 0);
  assert(header_length[3] == (uint8_t) 6);

  uint8_t header_format_array[2] = {};
  uint16_t header_format = 0;

  for (int i = 0; i < sizeof(header_format_array); i++) {
    assert((fread(&header_format_array[i], sizeof(uint8_t),
            1, midi_pointer)) == 1);
  }

  assert((((header_format_array[1] == (unsigned) 0)
         || ((header_format_array[1] == (unsigned) 1))
         || ((header_format_array[1] == (unsigned) 2))))
         && ((header_format_array[0] == (unsigned) 0)));

  song->format = header_format_array[1];

  uint8_t header_num_tracks[2];
  uint16_t num_tracks = 0;

  for (int i = 0; i < sizeof(header_num_tracks); i++) {
    assert((fread(&header_num_tracks[i], sizeof(uint8_t), 1,
            midi_pointer)) == 1);
  }

  num_tracks = end_swap_16(header_num_tracks);

  song->num_tracks = num_tracks;

  uint8_t header_division_array[2];
  uint16_t header_division = 0;
  division_t update_division = {};

  uint8_t division_frames_per_sec = 0;
  uint8_t division_ticks_per_frame = 0;

  for (int i = 0; i < sizeof(header_division_array); i++) {
    assert((fread(&header_division_array[i], sizeof(uint8_t), 1,
            midi_pointer)) == 1);
  }

  header_division = end_swap_16(header_division_array);

  assert(((sizeof(header_format)) + (sizeof(header_num_tracks))
    + (sizeof(header_division))) == HEADER_SIZE_ASSERT);

  if ((header_division & HEADER_DIVISION_OPERATOR)
       == HEADER_DIVISION_OPERATOR) {
    update_division.uses_tpq = false;
    division_frames_per_sec = (header_division & VALUE_SHIFT);
    division_ticks_per_frame = (header_division << BIT_SHIFT_8)
                                & DIVISION_SHIFT;
    update_division.frames_per_sec = division_frames_per_sec;
    update_division.ticks_per_frame = division_ticks_per_frame;
  }
  else {
    update_division.uses_tpq = true;
    update_division.ticks_per_qtr = header_division;
  }

  song->division = update_division;

  return;
} /* parse_header() */

/*
 * This function reads a MIDI track chunk from the given file pointer and
 * update the given song_data_t track_list with the track_t extracteed from
 * the file.
 */

void parse_track(FILE *midi_pointer, song_data_t *song) {

  char track_chunk_type[4] = "";
  char expected_track_chunk_type[4] = "MTrk";

  for (int i = 0; i < sizeof(track_chunk_type); i++) {
    assert((fread(&track_chunk_type[i], sizeof(char), 1, midi_pointer)) == 1);
    assert(track_chunk_type[i] == expected_track_chunk_type[i]);
  }

  uint8_t track_length_array[4];

  for (int i = 0; i < sizeof(track_length_array); i++) {
    assert((fread(&track_length_array[i], sizeof(uint8_t),
            1, midi_pointer)) == 1);
  }

  uint32_t track_length = 0;
  track_length = end_swap_32(track_length_array);

  track_node_t *first_track = malloc(sizeof(track_node_t));
  assert(first_track != NULL);

  track_t *track = malloc(sizeof(track_t));
  assert(track != NULL);
  track->length = track_length;

  first_track->track = track;
  first_track->next_track = NULL;

  track_node_t *track_node = song->track_list;

  event_node_t *first_event = NULL;
  event_node_t *current_event = NULL;

  bool track_end = false;
  int count = 0;
  int meta_compare = 0;

  event_node_t *temp = NULL;

  while (track_end == false) {
    current_event = malloc(sizeof(event_node_t));
    assert(current_event != NULL);

    current_event->event = parse_event(midi_pointer);
    current_event->next_event = NULL;

    if (event_type(current_event->event) == META_EVENT_T) {
      meta_compare = strcmp(current_event->event->meta_event.name,
                            "End of Track");
      if (meta_compare == 0) {
        track_end = true;
      }
    }

    if (count == 0) {
      first_event = current_event;
      temp = current_event;
    }

    else {
      temp->next_event = current_event;
      temp = current_event;
    }

    count++;
  }

  first_track->track->event_list = first_event;

  if (track_node == NULL) {
    song->track_list = first_track;
    return;
  }

  while (track_node->next_track != NULL) {
    track_node = track_node->next_track;
  }

  first_track->next_track = NULL;
  track_node->next_track = first_track;

  return;
} /* parse_track() */

/*
 * This function reads and returns a pointer to an event_t struct from the
 * given MIDI file pointer.
 */

event_t *parse_event(FILE *midi_pointer) {

  event_t *update_event = NULL;
  update_event = (event_t *) calloc(1, sizeof(event_t));
  assert(update_event != NULL);

  uint32_t event_delta_time = 0;

  event_delta_time = parse_var_len(midi_pointer);
  update_event->delta_time = event_delta_time;

  uint8_t event_type = 0;
  int correct = 0;

  correct = fread(&event_type, sizeof(uint8_t), 1, midi_pointer);

  if (correct != 1) {
    return update_event;
  }

  if (event_type == SYS_EVENT_1) {
    update_event->type = SYS_EVENT_1;
    update_event->sys_event = parse_sys_event(midi_pointer, SYS_EVENT_1);
  }
  else if (event_type == SYS_EVENT_2) {
    update_event->type = SYS_EVENT_2;
    update_event->sys_event = parse_sys_event(midi_pointer, SYS_EVENT_2);
  }
  else if (event_type == META_EVENT) {
    update_event->type = META_EVENT;
    update_event->meta_event = parse_meta_event(midi_pointer);
  }
  else {
    update_event->midi_event = parse_midi_event(midi_pointer, event_type);
    update_event->type = g_status;
  }

  return update_event;
} /* parse_event() */

/*
 * This function reads and returns a sys_event_t struct from the given MIDI
 * file pointer.
 */

sys_event_t parse_sys_event(FILE *midi_pointer, uint8_t event_type) {
  sys_event_t sys_event = {};

  uint32_t sysex_data_len = 0;
  sysex_data_len = parse_var_len(midi_pointer);

  sys_event.data_len = sysex_data_len;

  uint8_t sysex_data_read = 0;
  uint8_t *sysex_data = 0;
  int count = 0;
  sysex_data = calloc(1, sizeof(uint8_t) * sysex_data_len);
  assert(sysex_data != NULL);

  while (count < sysex_data_len) {
    assert((fread(&sysex_data_read, sizeof(uint8_t), 1, midi_pointer)) == 1);
    *(sysex_data + count) = sysex_data_read;
    count++;
  }

  sys_event.data = sysex_data;

  return sys_event;
} /* parse_sys_event() */

/*
 * This function reads and returns a meta_event_t struct from the given MIDI
 * file pointer.
 */

meta_event_t parse_meta_event(FILE *midi_pointer) {

  meta_event_t meta_event = {};

  uint8_t meta_type = 0;

  assert((fread(&meta_type, sizeof(uint8_t), 1, midi_pointer)) == 1);

  assert(META_TABLE[meta_type].name != NULL);

  meta_event.name = META_TABLE[meta_type].name;
  meta_event.data_len = parse_var_len(midi_pointer);

  if (META_TABLE[meta_type].data_len > 0) {
    assert(meta_event.data_len == META_TABLE[meta_type].data_len);
  }

  if (meta_event.data_len > 0) {
    meta_event.data = (uint8_t*) calloc(meta_event.data_len, sizeof(uint8_t));
    assert(meta_event.data != NULL);

    for (int i = 0; i < meta_event.data_len; i++) {
      assert((fread(&meta_event.data[i], sizeof(uint8_t),
              1, midi_pointer)) == 1);
    }

  }

  return meta_event;
} /* parse_meta_event() */

/*
 * This function reads and returns a midi_event_t struct from the given MIDI
 * file pointer.
 */

midi_event_t parse_midi_event(FILE *midi_pointer, uint8_t status) {

  midi_event_t midi_event = {};

  if ((status >> BIT_SHIFT_7) & 1) {
    midi_event = MIDI_TABLE[status];
    assert(midi_event.name != NULL);
    midi_event.status = status;
    g_status = status;

    if (midi_event.data_len > 0) {
      midi_event.data = calloc(midi_event.data_len, sizeof(uint8_t));
      assert(midi_event.data != NULL);

      for (int i = 0; i < midi_event.data_len; i++) {
        assert((fread(&midi_event.data[i], sizeof(uint8_t),
                1, midi_pointer)) == 1);
      }

    }

  }
  else {
    midi_event = MIDI_TABLE[g_status];

    if (midi_event.data_len > 0) {
      midi_event.data = calloc(midi_event.data_len, sizeof(uint8_t));
      assert(midi_event.data != NULL);
      midi_event.data[0] = status;

      for (int i = 1; i < midi_event.data_len; i++) {
        assert((fread(&midi_event.data[i], sizeof(uint8_t),
                1, midi_pointer)) == 1);
      }
    }
  }

  return midi_event;
} /* parse_midi_event() */

/*
 * This function reads a variable length integer from the given MIDI file
 * pointer and returns it as a fixed-size uint32_t.
 */

uint32_t parse_var_len(FILE *midi_pointer) {

  uint32_t value = 0;
  uint8_t byte = 0;

  if ((value = getc(midi_pointer)) & BYTE_SHIFT) {
    value &= VALUE_SHIFT;
    do {
      value = value << BIT_SHIFT_7;
      byte = getc(midi_pointer);
      value += (byte & VALUE_SHIFT);
    } while (byte & BYTE_SHIFT);
  }

  return value;
} /* parse_var_len() */

/*
 * This function takes in a pointer to an event, and should return either
 * SYS_EVENT_T, META_EVENT_T, or MIDI_EVENT_T, based on the type of the event.
 */

uint8_t event_type(event_t *update_event) {
  uint8_t event_type = 0;

  if ((update_event->type == SYS_EVENT_1)
       || (update_event->type == SYS_EVENT_2)) {
    event_type = SYS_EVENT_T;
  }
  else if (update_event->type == META_EVENT) {
    event_type = META_EVENT_T;
  }
  else {
    event_type = MIDI_EVENT_T;
  }

  return event_type;
} /* event_type() */

/*
 * This function should free all memory associated with the given song.
 */

void free_song(song_data_t *song_data) {
  free(song_data->path);
  song_data->path = NULL;

  if (song_data != NULL) {
    if (song_data->track_list != NULL) {
      free_track_node(song_data->track_list);
    }
  }

  song_data->track_list = NULL;
  free(song_data);
  song_data = NULL;
} /* free_song() */

/*
 * This function should free all memory associated with the given track node.
 */

void free_track_node(track_node_t *track_node) {

  track_node_t *temp = NULL;
  temp = track_node;

  while (track_node) {
    temp = track_node->next_track;

    free_event_node(track_node->track->event_list);
    track_node->track->event_list = NULL;

    free(track_node->track);
    track_node->track = NULL;
    free(track_node);
    track_node = NULL;
    track_node = temp;
  }

  return;
} /* free_track_node() */

/*
 * This function should free all memory associated with the given event node.
 */

void free_event_node(event_node_t *event_node) {
  event_node_t *temp = NULL;
  temp = event_node;

  uint8_t type = 0;

  while (event_node) {
    temp = event_node->next_event;
    type = event_type(event_node->event);

    if (type == (unsigned int) 1) {
      free(event_node->event->sys_event.data);
      event_node->event->sys_event.data = NULL;
    }
    else if (type == (unsigned int) 2) {
      free(event_node->event->meta_event.data);
      event_node->event->meta_event.name = NULL;
    }
    else if (type == (unsigned int) 3) {
      free(event_node->event->midi_event.data);
      event_node->event->meta_event.name = NULL;
    }

    free(event_node->event);
    event_node->event = NULL;

    free(event_node);
    event_node = NULL;

    event_node = temp;
  }

  return;
} /* free_event_node() */

/*
 * This function takes in a buffer of two uint8_ts representing a uint32_t,
 * and should return a uint16_t with opposite endianness.
 */

uint16_t end_swap_16(uint8_t swap_values[2]) {
  return ((uint16_t)(swap_values[0] << 8 | (uint16_t)swap_values[1]) << 0);
} /* end_swap_16() */

/*
 * This function takes in a buffer of four uint8_ts representing a uint32_t,
 * and should return a uint32_t with opposite endianness.
 */

uint32_t end_swap_32(uint8_t swap_values[4]) {
  return ((uint32_t)swap_values[0] << BIT_SHIFT_24
           | (uint32_t)swap_values[1] << BIT_SHIFT_16
           | (uint32_t)swap_values[2] << BIT_SHIFT_8
           | (uint32_t)swap_values[3] << 0);
} /* end_swap_32() */
