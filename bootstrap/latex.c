#include "global.h"
static int scraps = 1;
static void copy_scrap();             /* formats the body of a scrap */
static void print_scrap_numbers();      /* formats a list of scrap numbers */
static void format_entry();             /* formats an index entry */
static void format_user_entry();
void write_tex(file_name, tex_name)
     char *file_name;
     char *tex_name;
{
  FILE *tex_file = fopen(tex_name, "w");
  if (tex_file) {
    if (verbose_flag)
      fprintf(stderr, "writing %s\n", tex_name);
    source_open(file_name);
    if (hyperref_flag == TRUE) {
      fputs("\\newcommand{\\NWtarget}[2]{\\hypertarget{#1}{#2}}\n", tex_file);
      fputs("\\newcommand{\\NWlink}[2]{\\hyperlink{#1}{#2}}\n", tex_file);
    } else {
      fputs("\\newcommand{\\NWtarget}[2]{#2}\n", tex_file);
      fputs("\\newcommand{\\NWlink}[2]{#2}\n", tex_file);
    }

    fputs("\\newcommand{\\NWtxtMacroDefBy}{Macro defined by}\n", tex_file);
    fputs("\\newcommand{\\NWtxtMacroRefIn}{Macro referenced in}\n", tex_file);
    fputs("\\newcommand{\\NWtxtMacroNoRef}{Macro never referenced}\n", tex_file);
    fputs("\\newcommand{\\NWtxtDefBy}{Defined by}\n", tex_file);
    fputs("\\newcommand{\\NWtxtRefIn}{Referenced in}\n", tex_file);
    fputs("\\newcommand{\\NWtxtNoRef}{Not referenced}\n", tex_file);
    fputs("\\newcommand{\\NWtxtFileDefBy}{File defined by}\n", tex_file);
    fputs("\\newcommand{\\NWtxtIdentsUsed}{Uses:}\n", tex_file);
    fputs("\\newcommand{\\NWtxtIdentsNotUsed}{Never used}\n", tex_file);
    fputs("\\newcommand{\\NWtxtIdentsDefed}{Defines:}\n", tex_file);
    fputs("\\newcommand{\\NWsep}{${\\diamond}$}\n", tex_file);
    
    {
      int c = source_get();
      while (c != EOF) {
        if (c == nw_char)
          {
          {
            int big_definition = FALSE;
            c = source_get();
            switch (c) {
              case 'r':
                    c = source_get();
                    nw_char = c;
                    update_delimit_scrap();
                    break;
              case 'O': big_definition = TRUE;
              case 'o': {
                          Name *name = collect_file_name();
                          {
                            fputs("\\begin{flushleft} \\small", tex_file);
                            if (!big_definition)
                              fputs("\n\\begin{minipage}{\\linewidth}", tex_file);
                            fprintf(tex_file, " \\label{scrap%d}\n", scraps);
                          }
                          fprintf(tex_file, "\\verb%c\"%s\"%c\\nobreak\\ {\\footnotesize ", nw_char, name->spelling, nw_char);
                          fputs("\\NWtarget{nuweb", tex_file);
                          write_single_scrap_ref(tex_file, scraps);
                          fputs("}{", tex_file);
                          write_single_scrap_ref(tex_file, scraps);
                          fputs("}", tex_file);
                          fputs(" }$\\equiv$\n", tex_file);
                          {
                            fputs("\\vspace{-1ex}\n\\begin{list}{}{} \\item\n", tex_file);
                            copy_scrap(tex_file, TRUE);
                            fputs("{\\NWsep}\n\\end{list}\n", tex_file);
                          }
                          if ( scrap_flag ) {
                            {
                              if (name->defs->next) {
                                fputs("\\vspace{-1ex}\n", tex_file);
                                fputs("\\footnotesize\\addtolength{\\baselineskip}{-1ex}\n", tex_file);
                                fputs("\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}", tex_file);
                                fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
                                fputs("\\item \\NWtxtFileDefBy\\ ", tex_file);
                                print_scrap_numbers(tex_file, name->defs);
                                fputs("\\end{list}\n", tex_file);
                              }
                              else
                                fputs("\\vspace{-2ex}\n", tex_file);
                            }
                          }
                          {
                            format_defs_refs(tex_file, scraps);
                            format_uses_refs(tex_file, scraps++);
                            if (!big_definition)
                              fputs("\\end{minipage}\\\\[4ex]\n", tex_file);
                            fputs("\\end{flushleft}\n", tex_file);
                            do
                              c = source_get();
                            while (isspace(c));
                          }
                        }
                        break;
              case 'D': big_definition = TRUE;
              case 'd': {
                          Name *name = collect_macro_name();
                          {
                            fputs("\\begin{flushleft} \\small", tex_file);
                            if (!big_definition)
                              fputs("\n\\begin{minipage}{\\linewidth}", tex_file);
                            fprintf(tex_file, " \\label{scrap%d}\n", scraps);
                          }
                          fprintf(tex_file, "$\\langle\\,${\\it %s}\\nobreak\\ {\\footnotesize ", name->spelling);
                          fputs("\\NWtarget{nuweb", tex_file);
                          write_single_scrap_ref(tex_file, scraps);
                          fputs("}{", tex_file);
                          write_single_scrap_ref(tex_file, scraps);
                          fputs("}", tex_file);
                          fputs("}$\\,\\rangle\\equiv$\n", tex_file);
                          {
                            fputs("\\vspace{-1ex}\n\\begin{list}{}{} \\item\n", tex_file);
                            copy_scrap(tex_file, TRUE);
                            fputs("{\\NWsep}\n\\end{list}\n", tex_file);
                          }
                          {
                            fputs("\\vspace{-1ex}\n", tex_file);
                            fputs("\\footnotesize\\addtolength{\\baselineskip}{-1ex}\n", tex_file);
                            fputs("\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}", tex_file);
                            fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
                            if (name->defs->next) {
                              fputs("\\item \\NWtxtMacroDefBy\\ ", tex_file);
                              print_scrap_numbers(tex_file, name->defs);
                            }
                          }
                          {
                            if (name->uses) {
                              if (name->uses->next) {
                                fputs("\\item \\NWtxtMacroRefIn\\ ", tex_file);
                                print_scrap_numbers(tex_file, name->uses);
                              }
                              else {
                                fputs("\\item \\NWtxtMacroRefIn\\ ", tex_file);
                                fputs("\\NWlink{nuweb", tex_file);
                                write_single_scrap_ref(tex_file, name->uses->scrap);
                                fputs("}{", tex_file);
                                write_single_scrap_ref(tex_file, name->uses->scrap);
                                fputs("}", tex_file);
                                fputs(".\n", tex_file);
                              }
                            }
                            else {
                              fputs("\\item {\\NWtxtMacroNoRef}.\n", tex_file);
                              fprintf(stderr, "%s: <%s> never referenced.\n",
                                      command_name, name->spelling);
                            }
                            fputs("\\end{list}\n", tex_file);
                          }
                          {
                            format_defs_refs(tex_file, scraps);
                            format_uses_refs(tex_file, scraps++);
                            if (!big_definition)
                              fputs("\\end{minipage}\\\\[4ex]\n", tex_file);
                            fputs("\\end{flushleft}\n", tex_file);
                            do
                              c = source_get();
                            while (isspace(c));
                          }
                        }
                        break;
              case 's':
                        
                        current_sector += 1;
                        do
                           c = source_get();
                        while (isspace(c));
                        
                        break;
              case '{':
              case '[':
              case '(': copy_scrap(tex_file, FALSE);
                        c = source_get();
                        
                        break;
              case 'x': {
                           char  label_name[100];
                           char * p = label_name;
                           while (c = source_get(), c != nw_char) /* Here is @xlabel@x */
                              *p++ = c;
                           *p = '\0';
                           c = source_get();
                           
                           write_label(label_name, tex_file);
                        }
                        break;
              case 'f': {
                          if (file_names) {
                            fputs("\n{\\small\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}",
                                  tex_file);
                            fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
                            format_entry(file_names, tex_file, TRUE, 0);
                            fputs("\\end{list}}", tex_file);
                          }
                          c = source_get();
                        }
                        break;
              case 'm': {
                          if (macro_names) {
                            unsigned char sector = current_sector;
                            int c = source_get();
                            if (c == '+')
                               sector = 0;
                            else
                               source_ungetc(&c);

                            fputs("\n{\\small\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}",
                                  tex_file);
                            fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
                            format_entry(macro_names, tex_file, FALSE, sector);
                            fputs("\\end{list}}", tex_file);
                          }
                          c = source_get();
                        }
                        break;
              case 'u': {
                            unsigned char sector = current_sector;
                            c = source_get();
                            if (c == '+') {
                               sector = 0;
                               c = source_get();
                            }
                            if (has_sector(user_names, sector)) {
                               fputs("\n{\\small\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}",
                                     tex_file);
                               fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
                                 format_user_entry(user_names, tex_file, sector);
                               fputs("\\end{list}}", tex_file);
                            }
                        }
                        break;
              default:  
                    if (c==nw_char)
                      putc(c, tex_file); 
                    c = source_get();
                        break;
            }
          }
          }
        else {
          putc(c, tex_file);
          c = source_get();
        }
      }
    }
    fclose(tex_file);
  }
  else
    fprintf(stderr, "%s: can't open %s\n", command_name, tex_name);
}
static void print_scrap_numbers(tex_file, scraps)
     FILE *tex_file;
     Scrap_Node *scraps;
{
  int page;
  fputs("\\NWlink{nuweb", tex_file);
  write_scrap_ref(tex_file, scraps->scrap, -1, &page);
  fputs("}{", tex_file);
  write_scrap_ref(tex_file, scraps->scrap, TRUE, &page);
  fputs("}", tex_file);
  scraps = scraps->next;
  while (scraps) {
    fputs("\\NWlink{nuweb", tex_file);
    write_scrap_ref(tex_file, scraps->scrap, -1, &page);
    fputs("}{", tex_file);
    write_scrap_ref(tex_file, scraps->scrap, FALSE, &page);
    scraps = scraps->next;
    fputs("}", tex_file);
  }
  fputs(".\n", tex_file);
}
static char *orig_delimit_scrap[3][5] = {
  /* {} mode: begin, end, insert nw_char, prefix, suffix */
  { "\\verb@", "@", "@{\\tt @}\\verb@", "\\mbox{}", "\\\\" },
  /* [] mode: begin, end, insert nw_char, prefix, suffix */
  { "", "", "@", "", "" },
  /* () mode: begin, end, insert nw_char, prefix, suffix */
  { "$", "$", "@", "", "" },
};

static char *delimit_scrap[3][5];
void initialise_delimit_scrap_array() {
  int i,j;
  for(i = 0; i < 3; i++) {
    for(j = 0; j < 5; j++) {
      if((delimit_scrap[i][j] = strdup(orig_delimit_scrap[i][j])) == NULL) {
        fprintf(stderr, "Not enough memory for string allocation\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  /* replace verb by lstinline */
  if (listings_flag == TRUE) {
    free(delimit_scrap[0][0]);
    if((delimit_scrap[0][0]=strdup("\\lstinline@")) == NULL) {
      fprintf(stderr, "Not enough memory for string allocation\n");
      exit(EXIT_FAILURE);
    }
    free(delimit_scrap[0][2]);
    if((delimit_scrap[0][2]=strdup("@{\\tt @}\\lstinline@")) == NULL) {
      fprintf(stderr, "Not enough memory for string allocation\n");
      exit(EXIT_FAILURE);
    }
  }
}
int scrap_type = 0;

static void copy_scrap(file, prefix)
     FILE *file;
     int prefix;
{
  int indent = 0;
  int c;
  if (source_last == '{') scrap_type = 0;
  if (source_last == '[') scrap_type = 1;
  if (source_last == '(') scrap_type = 2;
  c = source_get();
  if (prefix) fputs(delimit_scrap[scrap_type][3], file);
  fputs(delimit_scrap[scrap_type][0], file);
  while (1) {
    switch (c) {
      case '\n': fputs(delimit_scrap[scrap_type][1], file);
                 if (prefix) fputs(delimit_scrap[scrap_type][4], file);
                 fputs("\n", file);
                 if (prefix) fputs(delimit_scrap[scrap_type][3], file);
                 fputs(delimit_scrap[scrap_type][0], file);
                 indent = 0;
                 break;
      case '\t': {
                   int delta = 3 - (indent % 3);
                   indent += delta;
                   while (delta > 0) {
                     putc(' ', file);
                     delta--;
                   }
                 }
                 break;
      default:   
         if (c==nw_char)
           {
             {
               c = source_get();
               switch (c) {
                 case 'x': {
                              char  label_name[100];
                              char * p = label_name;
                              while (c = source_get(), c != nw_char) /* Here is @xlabel@x */
                                 *p++ = c;
                              *p = '\0';
                              c = source_get();
                              
                              write_label(label_name, file);
                           }
                           break;
                 case '+':
                 case '-':
                 case '|': {
                             do {
                               do
                                 c = source_get();
                               while (c != nw_char);
                               c = source_get();
                             } while (c != '}' && c != ']' && c != ')' );
                           }
                 case ',':
                 case ')':
                 case ']':
                 case '}': fputs(delimit_scrap[scrap_type][1], file);
                           return;
                 case '<': {
                             Name *name = collect_scrap_name();
                             fputs(delimit_scrap[scrap_type][1],file);
                             if (prefix)
                               fputs("\\hbox{", file);
                             fprintf(file, "$\\langle\\,${\\it %s}\\nobreak\\ ", name->spelling);
                             if (scrap_name_has_parameters) {
                               
                                  char sep;

                                  sep = '(';
                                  do {
                                    fputc(sep,file);

                                    fputs("{\\footnotesize ", file);
                                    if (arglabels_flag)     
                                      write_single_scrap_ref(file, scraps + 1);
                                    fprintf(file, "\\label{scrap%d}\n", scraps + 1);
                                    fputs(" }", file);

                                    source_last = '{';
                                    copy_scrap(file, TRUE);

                                    ++scraps;

                                    sep = ',';
                                  } while ( source_last != ')' && source_last != EOF );
                                  fputs(" ) ",file);
                                  do 
                                    c = source_get();
                                  while(c != nw_char && c != EOF);
                                  if (c == nw_char) {
                                    c = source_get();
                                  }
                               
                             }
                             fprintf(file, "{\\footnotesize ");
                             if (name->defs)
                               {
                                 Scrap_Node *p = name->defs;
                                 fputs("\\NWlink{nuweb", file);
                                 write_single_scrap_ref(file, p->scrap);
                                 fputs("}{", file);
                                 write_single_scrap_ref(file, p->scrap);
                                 fputs("}", file);
                                 p = p->next;
                                 if (p)
                                   fputs(", \\ldots\\ ", file);
                               }
                             else {
                               putc('?', file);
                               fprintf(stderr, "%s: never defined <%s>\n",
                                       command_name, name->spelling);
                             }
                             fputs("}$\\,\\rangle$", file);
                             if (prefix)
                                fputs("}", file);
                             fputs(delimit_scrap[scrap_type][0], file);
                           }
                           break;
                 case '%': {
                                   do
                                           c = source_get();
                                   while (c != '\n');
                           }
                           break;
                 case '_': {
                             fputs(delimit_scrap[scrap_type][1],file);
                             fprintf(file, "\\hbox{\\sffamily\\bfseries ");
                             c = source_get();
                             do {
                                 fputc(c, file);
                                 c = source_get();
                             } while (c != nw_char);
                             c = source_get();
                             fprintf(file, "}");
                             fputs(delimit_scrap[scrap_type][0], file);
                           }
                           break;
                 case 'f': {
                             fputs(delimit_scrap[scrap_type][1],file);
                             fprintf(file, "\\hbox{\\sffamily\\slshape file name}");
                             fputs(delimit_scrap[scrap_type][0], file);
                           }
                           break;
                 case '1': case '2': case '3': 
                 case '4': case '5': case '6': 
                 case '7': case '8': case '9': 
                           fputs(delimit_scrap[scrap_type][1], file);
                           fputc(nw_char, file);
                           fputc(c,   file);
                           fputs(delimit_scrap[scrap_type][0], file);
                           break;
                 default:  
                       if (c==nw_char)
                         {
                           fputs(delimit_scrap[scrap_type][2], file);
                           break;
                         }
                       /* ignore these since pass1 will have warned about them */
                           break;
               }
             }
             break;
           }           
         putc(c, file);
                 indent++;
                 break;
    }
    c = source_get();
  }
}
void update_delimit_scrap()
{
  static int been_here_before = 0;

  /* {}-mode begin */
  if (listings_flag == TRUE) {
    delimit_scrap[0][0][10] = nw_char;
  } else {
    delimit_scrap[0][0][5] = nw_char;
  }
  /* {}-mode end */
  delimit_scrap[0][1][0] = nw_char;
  /* {}-mode insert nw_char */

  delimit_scrap[0][2][0] = nw_char;
  delimit_scrap[0][2][6] = nw_char;

  if (listings_flag == TRUE) {
    delimit_scrap[0][2][18] = nw_char;
  } else {
    delimit_scrap[0][2][13] = nw_char;
  }  

  /* []-mode insert nw_char */
  delimit_scrap[1][2][0] = nw_char;

  /* ()-mode insert nw_char */
  delimit_scrap[2][2][0] = nw_char;
}
static void format_entry(name, tex_file, file_flag, sector)
     Name *name;
     FILE *tex_file;
     int file_flag;
     unsigned char sector;
{
  while (name) {
    format_entry(name->llink, tex_file, file_flag, sector);
    if (name->sector == sector){
      fputs("\\item ", tex_file);
      if (file_flag) {
        fprintf(tex_file, "\\verb%c\"%s\"%c ", nw_char, name->spelling, nw_char);
        {
          Scrap_Node *p = name->defs;
          fputs("{\\footnotesize {\\NWtxtDefBy}", tex_file);
          if (p->next) {
            /* fputs("s ", tex_file); */
              putc(' ', tex_file);
            print_scrap_numbers(tex_file, p);
          }
          else {
            putc(' ', tex_file);
            fputs("\\NWlink{nuweb", tex_file);
            write_single_scrap_ref(tex_file, p->scrap);
            fputs("}{", tex_file);
            write_single_scrap_ref(tex_file, p->scrap);
            fputs("}", tex_file);
            putc('.', tex_file);
          }
          putc('}', tex_file);
        }
      }
      else {
        fprintf(tex_file, "$\\langle\\,$%s\\nobreak\\ {\\footnotesize ", name->spelling);
        {
          Scrap_Node *p = name->defs;
          if (p) {
            int page;
            fputs("\\NWlink{nuweb", tex_file);
            write_scrap_ref(tex_file, p->scrap, -1, &page);
            fputs("}{", tex_file);
            write_scrap_ref(tex_file, p->scrap, TRUE, &page);
            fputs("}", tex_file);
            p = p->next;
            while (p) {
              fputs("\\NWlink{nuweb", tex_file);
              write_scrap_ref(tex_file, p->scrap, -1, &page);
              fputs("}{", tex_file);
              write_scrap_ref(tex_file, p->scrap, FALSE, &page);
              fputs("}", tex_file);
              p = p->next;
            }
          }
          else
            putc('?', tex_file);
        }
        fputs("}$\\,\\rangle$ ", tex_file);
        {
          Scrap_Node *p = name->uses;
          fputs("{\\footnotesize ", tex_file);
          if (p) {
            fputs("{\\NWtxtRefIn}", tex_file);
            if (p->next) {
              /* fputs("s ", tex_file); */
              putc(' ', tex_file);
              print_scrap_numbers(tex_file, p);
            }
            else {
              putc(' ', tex_file);
              fputs("\\NWlink{nuweb", tex_file);
              write_single_scrap_ref(tex_file, p->scrap);
              fputs("}{", tex_file);
              write_single_scrap_ref(tex_file, p->scrap);
              fputs("}", tex_file);
              putc('.', tex_file);
            }
          }
          else
            fputs("{\\NWtxtNoRef}.", tex_file);
          putc('}', tex_file);
        }
      }
      putc('\n', tex_file);
    }
    name = name->rlink;
  }
}
int has_sector(name, sector)
Name * name;
unsigned char sector;
{
  while(name) {
    if (name->sector == sector)
       return TRUE;
    if (has_sector(name->llink, sector))
       return TRUE;
     name = name->rlink;
   }
   return FALSE;
}
static void format_user_entry(name, tex_file, sector)
     Name *name;
     FILE *tex_file;
     unsigned char sector;
{
  while (name) {
    format_user_entry(name->llink, tex_file, sector);
    if (name->sector == sector){
      Scrap_Node *uses = name->uses;
      if ( uses || dangling_flag ) {
        int page;
        Scrap_Node *defs = name->defs;
        fprintf(tex_file, "\\item \\verb%c%s%c: ", nw_char,name->spelling,nw_char);
        if (!uses) {
            fputs("(\\underline{", tex_file);
            fputs("\\NWlink{nuweb", tex_file);
            write_single_scrap_ref(tex_file, defs->scrap);
            fputs("}{", tex_file);
            write_single_scrap_ref(tex_file, defs->scrap);
            fputs("})}", tex_file);
            page = -2;
            defs = defs->next;        
        }
        else
          if (uses->scrap < defs->scrap) {    
          fputs("\\NWlink{nuweb", tex_file);
          write_scrap_ref(tex_file, uses->scrap, -1, &page);
          fputs("}{", tex_file);
          write_scrap_ref(tex_file, uses->scrap, TRUE, &page);
          fputs("}", tex_file);
          uses = uses->next;
        }
        else {
          if (defs->scrap == uses->scrap)
            uses = uses->next;
          fputs("\\underline{", tex_file);

          fputs("\\NWlink{nuweb", tex_file);
          write_single_scrap_ref(tex_file, defs->scrap);
          fputs("}{", tex_file);
          write_single_scrap_ref(tex_file, defs->scrap);
          fputs("}}", tex_file);
          page = -2;
          defs = defs->next;
        }
        while (uses || defs) {
          if (uses && (!defs || uses->scrap < defs->scrap)) {
            fputs("\\NWlink{nuweb", tex_file);
            write_scrap_ref(tex_file, uses->scrap, -1, &page);
            fputs("}{", tex_file);
            write_scrap_ref(tex_file, uses->scrap, FALSE, &page);
            fputs("}", tex_file);
            uses = uses->next;
          }
          else {
            if (uses && defs->scrap == uses->scrap)
              uses = uses->next;
            fputs(", \\underline{", tex_file);

            fputs("\\NWlink{nuweb", tex_file);
            write_single_scrap_ref(tex_file, defs->scrap);
            fputs("}{", tex_file);
            write_single_scrap_ref(tex_file, defs->scrap);
            fputs("}", tex_file);
     
            putc('}', tex_file);
            page = -2;
            defs = defs->next;
          }
        }
        fputs(".\n", tex_file);
      }
    }
    name = name->rlink;
  }
}
