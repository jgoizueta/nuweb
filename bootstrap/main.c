#include "global.h"


#if defined(VMS)
#define PATH_SEP(c) (c==']'||c==':')
#define PATH_SEP_CHAR ""
#define DEFAULT_PATH ""
#elif defined(MSDOS)
#define PATH_SEP(c) (c=='\\')
#define PATH_SEP_CHAR "\\"
#define DEFAULT_PATH "."
#else
#define PATH_SEP(c) (c=='/')
#define PATH_SEP_CHAR "/"
#define DEFAULT_PATH "."
#endif
#include <stdlib.h>

int main(argc, argv)
     int argc;
     char **argv;
{
  int arg = 1;
  
  
  command_name = argv[0];
  while (arg < argc) {
    char *s = argv[arg];
    if (*s++ == '-') {
      {
        char c = *s++;
        while (c) {
          switch (c) {
            case 'c': compare_flag = FALSE;
                      break;
            case 'd': dangling_flag = TRUE;
                      break;
            case 'n': number_flag = TRUE;
                      break;
            case 'o': output_flag = FALSE;
                      break;
            case 's': scrap_flag = FALSE;
                      break;
            case 't': tex_flag = FALSE;
                      break;
            case 'v': verbose_flag = TRUE;
                      break;
            case 'p': prepend_flag = TRUE;
                      break;
            case 'l': listings_flag = TRUE;
                      break;
            case 'r': hyperref_flag = TRUE;
                      break;
            case 'u': undefined_flag = TRUE;
                      break;
            case 'a': arglabels_flag = TRUE;
                      break;
            default:  fprintf(stderr, "%s: unexpected argument ignored.  ",
                              command_name);
                      fprintf(stderr, "Usage is: %s [-clnortv] [-p path] file...\n",
                              command_name);
                      break;
          }
          c = *s++;
        }
      }
      arg++;
      if (prepend_flag)
      {
        dirpath = argv[arg++];
        prepend_flag = FALSE;
      }
      
    }
    else break;
  }
  
  {
    /* try to get locale information */
    char *s=getenv("LC_CTYPE");
    if (s==NULL) s=getenv("LC_ALL");

    /* set it */
    if (s!=NULL)
      if(setlocale(LC_CTYPE, s)==NULL)
        fprintf(stderr, "Setting locale failed\n");
  }
  
  initialise_delimit_scrap_array();
  {
    if (arg >= argc) {
      fprintf(stderr, "%s: expected a file name.  ", command_name);
      fprintf(stderr, "Usage is: %s [-clnortv] [-p path] file-name...\n", command_name);
      exit(-1);
    }
    do {
      {
        char source_name[FILENAME_MAX];
        char tex_name[FILENAME_MAX];
        char aux_name[FILENAME_MAX];
        {
          char *p = argv[arg];
          char *q = source_name;
          char *trim = q;
          char *dot = NULL;
          char c = *p++;
          while (c) {
            *q++ = c;
            if (PATH_SEP(c)) {
              trim = q;
              dot = NULL;
            }
            else if (c == '.')
              dot = q - 1;
            c = *p++;
          }
          *q = '\0';
          if (dot) {
            *dot = '\0'; /* produce HTML when the file extension is ".hw" */
            html_flag = dot[1] == 'h' && dot[2] == 'w' && dot[3] == '\0';
            sprintf(tex_name, "%s%s%s.tex", dirpath, path_sep, trim);
            sprintf(aux_name, "%s%s%s.aux", dirpath, path_sep, trim);
            *dot = '.';
          }
          else {
            sprintf(tex_name, "%s%s%s.tex", dirpath, path_sep, trim);
            sprintf(aux_name, "%s%s%s.aux", dirpath, path_sep, trim);
            *q++ = '.';
            *q++ = 'w';
            *q = '\0';
          }
        }
        {
          pass1(source_name);
          current_sector = 1;
          if (tex_flag) {
            if (html_flag) {
              int saved_number_flag = number_flag; 
              number_flag = TRUE;
              collect_numbers(aux_name);
              write_html(source_name, tex_name, 0/*Dummy */);
              number_flag = saved_number_flag;
            }
            else {
              collect_numbers(aux_name);
              write_tex(source_name, tex_name);
            }
          }
          if (output_flag)
            write_files(file_names);
          arena_free();
        }
      }
      arg++;
    } while (arg < argc);
  }
  exit(0);
}
