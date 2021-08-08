/* Jwala Aram Mitra, alterations.c, CS 24000, Spring 2020
 * Last updated April 9, 2020
 */

/* Add any includes here */

#include "alterations.h"

/*Define change_event_octave here */
int change_event_octave(event_t *event, int *octaves) {

  int modified = 0;

  /*uint8_t *event_data = event->midi_event->data;
  uint8_t new_data = event_data + (octaves * OCTAVE_STEP);

  if (((event->type & 0x90) == 0x90)
        || ((event->type & 0x80) == 0x80)
        || ((event->type & 0xA0) == 0xA0)) {
    if ((new_data < 0) || (new_data > 127)) {
      return modified;
    }
    else {
      event->midi_event->data = event->midi_event->new_data;
      return modified++;
    }*/

  return modified;
}

/*Define change_event_time here */
int change_event_time(event_t *event, float *multiplier) {

  // do I need to call parse event/parse var len to first parse the event?

  /*uint32_t original_dt = 0;
  original_dt = event->delta_time;

  (float) event->delta_time = (multiplier) * (float) event->delta_time;*/

  int difference = 0;

  //difference = event->delta_time - original_dt;

  return difference;
}

/*Define change_event_instrument here */

/*Define change_event_note here */

/*Define apply_to_events here */
int apply_to_event(song_data_t *song, event_func_t event_pointer, void *data) {

  int sum = 0;

  track_t *current_track = NULL;
  current_track = song->track_list->track;



  event_t *current_event = NULL;
  current_event = current_track->event_list->event;

  while (current_track != NULL) {

    while (current_event != NULL) {
      sum += (event_pointer)(current_event, data);
      current_event = current_event->next_event;
    }
    current_track = current_track->next_track;
    current_event = current_track->event_list->event;
  }

  return sum;
}

/*Define change_octave here */
int change_octave(song_data_t *song, int octaves) {
  return 0;
}

/*Define warp_time here */
int warp_time(song_data_t *song, float multiplier) {
  return 0;
}

/*Define remap_instruments here */
int remap_instruments(song_data_t *song, remapping_t map) {
  return 0;
}

/*Define remap_notes here */
int remap_notes(song_data_t *song, remapping_t map) {
  return 0;
}

/*Define add_round here */
void add_round(song_data_t *song, int a, int b, unsigned int c, uint8_t d) {
  return;
}

/*
 * Function called prior to main that sets up random mapping tables
 */

void build_mapping_tables()
{
  for (int i = 0; i <= 0xFF; i++) {
    I_BRASS_BAND[i] = 61;
  }

  for (int i = 0; i <= 0xFF; i++) {
    I_HELICOPTER[i] = 125;
  }

  for (int i = 0; i <= 0xFF; i++) {
    N_LOWER[i] = i;
  }
  //  Swap C# for C
  for (int i = 1; i <= 0xFF; i += 12) {
    N_LOWER[i] = i-1;
  }
  //  Swap F# for G
  for (int i = 6; i <= 0xFF; i += 12) {
    N_LOWER[i] = i+1;
  }
} /* build_mapping_tables() */
