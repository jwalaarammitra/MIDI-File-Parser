/* Jwala Aram Mitra, library.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#include "library.h"

#include <malloc.h>

#include <string.h>

#include "parser.h"

tree_node_t *g_song_library = NULL;

/*
 * This function locates the node in the given tree having the given song_name.
 */

tree_node_t **find_parent_pointer(tree_node_t **root, const char *song_name) {
  tree_node_t **end_tree = NULL;

  tree_node_t *parent = NULL;
  parent = *root;

  tree_node_t *temp = NULL;
  temp = *root;

  int song_compare = 0;

  if (strcmp((*root)->song_name, song_name) == 0) {
    return root;
  }

  while (true) {
    song_compare = strcmp(song_name, temp->song_name);

    if (song_compare == 0) {
      if (parent->left_child == temp) {
        return &(parent->left_child);
      }
      return &(parent->right_child);
    }
    else if (song_compare > 0) {
      if (temp->right_child == NULL) {
        break;
      }
      parent = temp;
      temp = temp->right_child;
    }
    else if (song_compare < 0) {
      if (temp->left_child == NULL) {
        break;
      }
      parent = temp;
      temp = temp->left_child;
    }
  }

  return end_tree;
} /* find_parent_pointer() */

/*
 * This function inserts a node into the tree unless it is already present.
 */

int tree_insert(tree_node_t **root, tree_node_t *to_insert) {
  tree_node_t *temp = NULL;
  temp = *root;

  char *key = (to_insert)->song_name;
  int song_compare = 0;

  if (*root == NULL) {
    *root = to_insert;
    return INSERT_SUCCESS;
  }

  while (true) {
    song_compare = strcmp(key, temp->song_name);

    if (song_compare == 0) {
      return DUPLICATE_SONG;
    }

    else if (song_compare > 0) {
      if (temp->right_child == NULL) {
        temp->right_child = to_insert;
        break;
      }
      temp = temp->right_child;
    }
    else if (song_compare < 0) {
      if (temp->left_child == NULL) {
        temp->left_child = to_insert;
        break;
      }
      temp = temp->left_child;
    }
  }

  return INSERT_SUCCESS;
} /* tree_insert() */

/*
 * This function searches the given tree for a song with the given name and
 * removes it if it is present.
 */

int remove_song_from_tree(tree_node_t **root, const char *song_name) {

  if (*root == NULL) {
    return SONG_NOT_FOUND;
  }

  tree_node_t *temp = NULL;
  temp = *root;

  tree_node_t *parent = NULL;
  parent = *root;

  tree_node_t *replace = NULL;

  tree_node_t *temp_left = NULL;
  tree_node_t *temp_right = NULL;

  tree_node_t *replace_parent = NULL;


  if ((root == NULL) || (*root == NULL)) {
    return SONG_NOT_FOUND;
  }


  int song_compare = 0;
  int node_compare = 0;
  int parent_compare = 0;
  int replace_parent_valid = 0;

  while (temp != NULL) {
    song_compare = strcmp(song_name, temp->song_name);

    if (song_compare == 0) {
      if ((temp->left_child == NULL) && (temp->right_child == NULL)) {

        if (temp == parent) {
          free_song(temp->song);
          temp->song = NULL;

          free(temp);
          temp = NULL;

          return DELETE_SUCCESS;
        }

        node_compare = strcmp(temp->song_name, parent->song_name);

        free_song(temp->song);
        temp->song = NULL;

        free(temp);
        temp = NULL;

        if (node_compare > 0) {
          parent->right_child = NULL;
        }

        if (node_compare < 0) {
          parent->left_child = NULL;
        }

        return DELETE_SUCCESS;
      }

      if ((temp->left_child == NULL) && (temp->right_child != NULL)) {

        if (temp == parent) {
          **root = *(temp->right_child);

          free_song(temp->song);
          temp->song = NULL;

          free(temp);
          temp = NULL;

          return DELETE_SUCCESS;
        }

        node_compare = strcmp(temp->right_child->song_name, parent->song_name);

        if (node_compare > 0) {
          parent->right_child = temp->right_child;
        }
        if (node_compare < 0) {
          parent->left_child = temp->right_child;
        }

        free_song(temp->song);
        temp->song = NULL;

        free(temp);
        temp->right_child = NULL;
        temp = NULL;
        return DELETE_SUCCESS;
      }

      if ((temp->left_child != NULL) && (temp->right_child == NULL)) {

        if (temp == parent) {
          **root = *(temp->left_child);

          free_song(temp->song);
          temp->song = NULL;

          free(temp);
          temp = NULL;

          return DELETE_SUCCESS;
        }

        node_compare = strcmp(temp->left_child->song_name, parent->song_name);

        if (node_compare > 0) {
          parent->right_child = temp->left_child;
        }
        if (node_compare < 0) {
          parent->left_child = temp->left_child;
        }

        free_song(temp->song);
        temp->song = NULL;

        free(temp);
        temp->left_child = NULL;
        temp = NULL;
        return DELETE_SUCCESS;
      }
      if ((temp->left_child != NULL) && (temp->right_child != NULL)) {

        if (temp == parent) {

          replace = temp->right_child;

          while (replace->left_child != NULL) {
            replace_parent_valid = 1;
            replace_parent = replace;
            replace = replace->left_child;
          }

          if (replace_parent_valid == 0) {
            replace->left_child = temp->left_child;

            free_song(temp->song);
            temp->song = NULL;

            free(temp);
            temp = NULL;

            **root = *replace;

            return DELETE_SUCCESS;
          }

          temp_left = temp->left_child;
          temp_right = temp->right_child;

          free_song(temp->song);
          temp->song = NULL;

          free(temp);
          temp = NULL;

          if (replace_parent_valid == 1) {
            replace_parent->left_child = NULL;
          }

          replace->left_child = temp_left;
          replace->right_child = temp_right;

          **root = *replace;

          return DELETE_SUCCESS;
        }

        parent_compare = strcmp(temp->song_name, parent->song_name);

        replace = temp->right_child;

        while (replace->left_child != NULL) {
          replace_parent_valid = 1;
          replace_parent = replace;
          replace = replace->left_child;
        }

        temp_left = temp->left_child;
        temp_right = temp->right_child;

        free_song(temp->song);
        temp->song = NULL;

        free(temp);
        temp = NULL;

        if (parent_compare < 0) {
          parent->left_child = replace;
        }

        if (parent_compare > 0) {
          parent->right_child = replace;
        }

        if (replace_parent_valid == 1) {
          replace_parent->left_child = NULL;
        }
        replace->left_child = temp_left;
        replace->right_child = temp_right;
        return DELETE_SUCCESS;
      }

    }

    else if (song_compare > 0) {
      parent = temp;
      temp = parent->right_child;
    }
    else if (song_compare < 0) {
      parent = temp;
      temp = parent->left_child;
    }

  }

  return SONG_NOT_FOUND;
} /* remove_song_from_tree() */

/*
 * This function frees all data associated with the given tree_node_t,
 * including the data associate with its song.
 */

void free_node(tree_node_t *tree_node) {

  if (tree_node == NULL) {
    return;
  }

  free_song(tree_node->song);
  tree_node->song = NULL;

  free(tree_node);
  tree_node = NULL;
} /* free_node() */

/*
 * This function prints the song name associate with the given tree node to the
 * specified file pointer, following by a newline character.
 */

void print_node(tree_node_t *tree_node, FILE *write_file) {
  if (tree_node->song_name == NULL) {
    return;
  }

  fprintf(write_file, "%s\n", tree_node->song_name);

  fclose(write_file);
  write_file = NULL;
} /* print_node() */

/*
 * This function preforms a pre-order traversal of the tree.
 */

void traverse_pre_order(tree_node_t *root, void *data,
                        traversal_func_t trav_pointer) {
  if (!root) {
    (trav_pointer)(root, data);
    traverse_in_order(root->left_child, data, trav_pointer);
    traverse_in_order(root->right_child, data, trav_pointer);
  }
} /* traverse_pre_order() */

/*
 * This function preforms an in-order traversal of the tree.
 */

void traverse_in_order(tree_node_t *root, void *data,
                       traversal_func_t trav_pointer) {
  if (!root) {
    traverse_in_order(root->left_child, data, trav_pointer);
    (trav_pointer)(root, data);
    traverse_in_order(root->right_child, data, trav_pointer);
  }
} /* traverse_in_order() */

/*
 * This function preforms a post-order traversal of the tree.
 */

void traverse_post_order(tree_node_t *root, void *data,
                         traversal_func_t trav_pointer) {
  if (!root) {
    traverse_post_order(root->left_child, data, trav_pointer);
    traverse_post_order(root->right_child, data, trav_pointer);
    (trav_pointer)(root, data);
  }
} /* traverse_post_order() */

/*
 * This function frees all data associated with the input tree.
 */

void free_library(tree_node_t *list) {
  if (list == NULL) {
    return;
  }

  tree_node_t *current = list;
  free_library(current->left_child);
  free_library(current->right_child);
  free_node(current);
} /* free_library() */

/*
 * This function prints the names of all songs in the given tree to the file,
 * separated by newline characters.
 */

void write_song_list(FILE *write_file, tree_node_t *root) {
} /* write_song_list() */

/*
 * This function takes in the name of a directory and seraches through the
 * directory structure, adding every MIDI file found in the structure to the
 * tree held by g_song_library.
 */

void make_library(const char *directory) {
} /* make_library() */
