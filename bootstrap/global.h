
/* #include <fcntl.h> */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <locale.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define MAX_INDENT 1500
typedef struct scrap_node {
  struct scrap_node *next;
  int scrap;
} Scrap_Node;
typedef struct name {
  char *spelling;
  struct name *llink;
  struct name *rlink;
  Scrap_Node *defs;
  Scrap_Node *uses;
  int mark;
  char tab_flag;
  char indent_flag;
  char debug_flag;
  unsigned char sector;
} Name;
typedef struct uses {
  struct uses *next;
  Name *defn;
} Uses;
typedef struct l_node
{
   struct l_node * left, * right;
   int scrap, seq;
   char name[1];
} label_node;

extern int tex_flag;      /* if FALSE, don't emit the documentation file */
extern int html_flag;     /* if TRUE, emit HTML instead of LaTeX scraps. */
extern int output_flag;   /* if FALSE, don't emit the output files */
extern int compare_flag;  /* if FALSE, overwrite without comparison */
extern int verbose_flag;  /* if TRUE, write progress information */
extern int number_flag;   /* if TRUE, use a sequential numbering scheme */
extern int scrap_flag;    /* if FALSE, don't print list of scraps */
extern int dangling_flag;    /* if FALSE, don't print dangling flags */
extern int prepend_flag;  /* If TRUE, prepend a path to the output file names */
extern char * dirpath;    /* The prepended directory path */
extern char * path_sep;   /* How to join path to filename */
extern int listings_flag;   /* if TRUE, use listings package for scrap formatting */
extern int hyperref_flag;   /* if TRUE, use hyperref formatting for targets and links */
extern int undefined_flag;  /* if TRUE, emit undefined macro names to output */
extern int arglabels_flag;  /* if TRUE, label macro parameters passed */
extern int nw_char;
extern char *command_name;
unsigned char current_sector;
extern char *source_name;  /* name of the current file */
extern int source_line;    /* current line in the source file */
extern int already_warned;
extern Name *file_names;
extern Name *macro_names;
extern Name *user_names;
extern int scrap_name_has_parameters;
extern int scrap_ended_with;
extern label_node * label_tab;

extern void pass1();
extern void write_tex();
void initialise_delimit_scrap_array(void);
void update_delimit_scrap();
extern void write_html();
extern void write_files();
extern void source_open(); /* pass in the name of the source file */
extern int source_get();   /* no args; returns the next char or EOF */
extern int source_last;   /* what last source_get() returned. */
extern void init_scraps();
extern int collect_scrap();
extern int write_scraps();
extern void write_scrap_ref();
extern void write_single_scrap_ref();
extern void collect_numbers();
extern Name *collect_file_name();
extern Name *collect_macro_name();
extern Name *collect_scrap_name();
extern Name *name_add();
extern Name *prefix_add();
extern char *save_string();
extern void reverse_lists();
extern void search();
void write_label(char label_name[], FILE * file);
extern void *arena_getmem();
extern void arena_free();

