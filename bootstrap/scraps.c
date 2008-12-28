#include "global.h"
typedef struct Param { int p[10]; struct Param* parent;} *Parameters;
#define SLAB_SIZE 500

typedef struct slab {
  struct slab *next;
  char chars[SLAB_SIZE];
} Slab;
typedef struct {
  char *file_name;
  Slab *slab;
  struct uses *uses;
  struct uses *defs;
  int file_line;
  int page;
  char letter;
  unsigned char sector;
} ScrapEntry;
static ScrapEntry *SCRAP[256];

#define scrap_array(i) SCRAP[(i) >> 8][(i) & 255]

static int scraps;
int delayed_indent = 0;

static void add_uses();
static int scrap_is_in();

void init_scraps()
{
  scraps = 1;
  SCRAP[0] = (ScrapEntry *) arena_getmem(256 * sizeof(ScrapEntry));
}
void write_scrap_ref(file, num, first, page)
     FILE *file;
     int num;
     int first;
     int *page;
{
  if (scrap_array(num).page >= 0) {
    if (first!=0)
      fprintf(file, "%d", scrap_array(num).page);
    else if (scrap_array(num).page != *page)
      fprintf(file, ", %d", scrap_array(num).page);
    if (scrap_array(num).letter > 0)
      fputc(scrap_array(num).letter, file);
  }
  else {
    if (first!=0)
      putc('?', file);
    else
      fputs(", ?", file);
    {
      if (!already_warned) {
        fprintf(stderr, "%s: you'll need to rerun nuweb after running latex\n",
                command_name);
        already_warned = TRUE;
      }
    }
  }
  if (first>=0)
  *page = scrap_array(num).page;
}
void write_single_scrap_ref(file, num)
     FILE *file;
     int num;
{
  int page;
  write_scrap_ref(file, num, TRUE, &page);
}
typedef struct {
  Slab *scrap;
  Slab *prev;
  int index;
} Manager;
static void push(c, manager)
     char c;
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  scrap->chars[index++] = c;
  if (index == SLAB_SIZE) {
    Slab *new = (Slab *) arena_getmem(sizeof(Slab));
    scrap->next = new;
    manager->scrap = new;
    index = 0;
  }
  manager->index = index;
}
static void pushs(s, manager)
     char *s;
     Manager *manager;
{
  while (*s)
    push(*s++, manager);
}
int collect_scrap()
{
  int current_scrap, lblseq = 0;
  int depth = 1;
  Manager writer;
  {
    Slab *scrap = (Slab *) arena_getmem(sizeof(Slab));
    if ((scraps & 255) == 0)
      SCRAP[scraps >> 8] = (ScrapEntry *) arena_getmem(256 * sizeof(ScrapEntry));
    scrap_array(scraps).slab = scrap;
    scrap_array(scraps).file_name = save_string(source_name);
    scrap_array(scraps).file_line = source_line;
    scrap_array(scraps).page = -1;
    scrap_array(scraps).letter = 0;
    scrap_array(scraps).uses = NULL;
    scrap_array(scraps).defs = NULL;
    scrap_array(scraps).sector = current_sector;
    writer.scrap = scrap;
    writer.index = 0;
    current_scrap = scraps++;
  }
  {
    int c = source_get();
    while (1) {
      switch (c) {
        case EOF: fprintf(stderr, "%s: unexpect EOF in (%s, %d)\n",
                          command_name, scrap_array(current_scrap).file_name,
                          scrap_array(current_scrap).file_line);
                  exit(-1);
        default:  
          if (c==nw_char)
            {
              {
                c = source_get();
                switch (c) {
                  case '(':
                  case '[':
                  case '{': depth++;
                            break;
                  case '+':
                  case '-':
                  case '|': {
                              do {
                                int type = c;
                                do {
                                char new_name[100];
                                char *p = new_name;
                                  unsigned int sector = 0;
                                do 
                                  c = source_get();
                                while (isspace(c));
                                if (c != nw_char) {
                                  Name *name;
                                  do {
                                    *p++ = c;
                                    c = source_get();
                                  } while (c != nw_char && !isspace(c));
                                  *p = '\0';
                                    switch (type) {
                                    case '-':
                                       name = name_add(&user_names, new_name, 0);
                                       if (!name->uses || name->uses->scrap != current_scrap) {
                                         Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         use->scrap = current_scrap;
                                         use->next = name->uses;
                                         name->uses = use;
                                         add_uses(&(scrap_array(current_scrap).uses), name);
                                       }
                                       /* Fall through */
                                    case '|': sector = current_sector;
                                       /* Fall through */
                                    case '+':
                                       name = name_add(&user_names, new_name, sector);
                                       if (!name->defs || name->defs->scrap != current_scrap) {
                                         Scrap_Node *def = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         def->scrap = current_scrap;
                                         def->next = name->defs;
                                         name->defs = def;
                                         add_uses(&(scrap_array(current_scrap).defs), name);
                                       }
                                       break;
                                  }
                                }
                              } while (c != nw_char);
                              c = source_get();
                              }while (c == '+' || c == '-' || c == '|');
                              if (c != '}' && c != ']' && c != ')') {
                                fprintf(stderr, "%s: unexpected %c%c in index entry (%s, %d)\n",
                                        command_name, nw_char, c, source_name, source_line);
                                exit(-1);
                              }
                            }
                            /* Fall through */
                  case ')':
                  case ']':
                  case '}': if (--depth > 0)
                              break;
                            /* else fall through */
                  case ',':
                            push('\0', &writer);
                            scrap_ended_with = c;
                            return current_scrap;
                  case '<': {
                              Name *name = collect_scrap_name();
                              {
                                char *s = name->spelling;
                                int len = strlen(s) - 1;
                                push(nw_char, &writer);
                                push('<', &writer);
                                push(name->sector, &writer);
                                while (len > 0) {
                                  push(*s++, &writer);
                                  len--;
                                }
                                if (*s == ' ')
                                  pushs("...", &writer);
                                else
                                  push(*s, &writer);
                              }
                              {
                                if (!name->uses || name->uses->scrap != current_scrap) {
                                  Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                  use->scrap = current_scrap;
                                  use->next = name->uses;
                                  name->uses = use;
                                }
                              }
                              if (scrap_name_has_parameters) {
                                {
                                  int param_scrap;
                                  char param_buf[10];

                                  push(nw_char, &writer);
                                  push('(', &writer);
                                  do {

                                     param_scrap = collect_scrap();
                                     sprintf(param_buf, "%d", param_scrap);
                                     pushs(param_buf, &writer);
                                     push(nw_char, &writer);
                                     push(scrap_ended_with, &writer);
                                     {
                                       if (!name->uses || name->uses->scrap != current_scrap) {
                                         Scrap_Node *use = (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
                                         use->scrap = current_scrap;
                                         use->next = name->uses;
                                         name->uses = use;
                                       }
                                     }
                                  } while( scrap_ended_with == ',' );
                                  do
                                    c = source_get();
                                  while( ' ' == c );
                                  if (c == nw_char) {
                                    c = source_get();
                                  }
                                  if (c != '>') {
                                    /* ZZZ print error */;
                                  }
                                }
                              }
                              push(nw_char, &writer);
                              push('>', &writer);
                              c = source_get();
                            }
                            break;
                  case '%': {
                                    do
                                            c = source_get();
                                    while (c != '\n');
                            }
                            /* emit line break to the output file to keep #line in sync. */
                            push('\n', &writer); 
                            c = source_get();
                            break;
                  case 'x': {
                               char  label_name[100];
                               char * p = label_name;
                               while (c = source_get(), c != nw_char) /* Here is @xlabel@x */
                                  *p++ = c;
                               *p = '\0';
                               c = source_get();
                               
                               if (label_name[0])
                               {
                                  label_node * * plbl = &label_tab;
                                  for (;;)
                                  {
                                     label_node * lbl = *plbl;

                                     if (lbl)
                                     {
                                        int cmp = label_name[0] - lbl->name[0];

                                        if (cmp == 0)
                                           cmp = strcmp(label_name + 1, lbl->name + 1);
                                        if (cmp < 0)
                                           plbl = &lbl->left;
                                        else if (cmp > 0)
                                           plbl = &lbl->right;
                                        else
                                        {
                                           fprintf(stderr, "Duplicate label %s.\n", label_name);
                                           break;
                                        }
                                     }
                                     else
                                     {
                                         lbl = (label_node *)arena_getmem(sizeof(label_node) + (p - label_name));
                                         lbl->left = lbl->right = NULL;
                                         strcpy(lbl->name, label_name);
                                         lbl->scrap = current_scrap;
                                         lbl->seq = ++lblseq;
                                         *plbl = lbl;
                                         break;
                                     }
                                  }
                               }
                               
                               else
                               {
                                  fprintf(stderr, "Empty label.\n");
                               }
                               push(nw_char, &writer);
                               push('x', &writer);
                               pushs(label_name, &writer);
                               push(nw_char, &writer);
                            }
                            break;
                  case '1': case '2': case '3':
                  case '4': case '5': case '6':
                  case '7': case '8': case '9':
                  case 'f': case '#':
                            push(nw_char, &writer);
                            break;
                  case '_': c = source_get();
                            break;
                  default : 
                        if (c==nw_char)
                          {
                            push(nw_char, &writer);
                            push(nw_char, &writer);
                            c = source_get();
                            break;
                          }
                        fprintf(stderr, "%s: unexpected %c%c in scrap (%s, %d)\n",
                                    command_name, nw_char, c, source_name, source_line);
                            exit(-1);
                }
              }
                  break;
            }
          push(c, &writer);
                  c = source_get();
                  break;
      }
    }
  }
}
static char pop(manager)
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  char c = scrap->chars[index++];
  if (index == SLAB_SIZE) {
    manager->prev = scrap;
    manager->scrap = scrap->next;
    index = 0;
  }
  manager->index = index;
  return c;
}
static void backup(n, manager)
     int n;
     Manager *manager;
{
  Slab *scrap = manager->scrap;
  int index = manager->index;
  if (n > index
      && manager->prev != NULL)
  {
     manager->scrap = manager->prev;
     manager->prev = NULL;
     index += SLAB_SIZE;
  }
  manager->index = (n <= index ? index - n : 0);
}
static Name *pop_scrap_name(manager, parameters)
     Manager *manager;
     Parameters *parameters;
{
  char name[100];
  char *p = name;
  int sector = pop(manager);
  int c = pop(manager);

  while (TRUE) {
    if (c == nw_char)
      {
        Name *pn;
        c = pop(manager);
        if (c == nw_char) {
          *p++ = c;
          c = pop(manager);
        }
        
          if (c == '(') {
            Parameters res = arena_getmem(sizeof(struct Param));
            int *p2 = res->p;
            int count = 0;
            int scrapnum;

            res->parent = 0;
            while( c && c != ')' ) {
              scrapnum = 0;
              c = pop(manager);
              while( '0' <= c && c <= '9' ) {
                scrapnum = scrapnum  * 10 + c - '0';
                c = pop(manager);
              }
              if ( c == nw_char ) {
                c = pop(manager);
              }
              *p2++ = scrapnum;
              count++;
            }
            while (count < 10) {
              *p2++ = 0;
              count++;
            }
            while( c && c != nw_char ) {
                c = pop(manager);
            }
            if ( c == nw_char ) {
              c = pop(manager);
            }
            *parameters = res;
          }
        else
          {
            Parameters res = arena_getmem(sizeof(*res));
            int *p2 = res->p;
            int count = 0;

            res->parent = 0;
            
            while (count < 10) {
              *p2++ = 0;
              count++;
            }
            *parameters = res;
          }
        
        if (c == '>') {
          if (p - name > 3 && p[-1] == '.' && p[-2] == '.' && p[-3] == '.') {
            p[-3] = ' ';
            p -= 2;
          }
          *p = '\0';
          pn = prefix_add(&macro_names, name, sector);
          return pn;
        }
        else {
          fprintf(stderr, "%s: found an internal problem (1)\n", command_name);
          exit(-1);
        }
      }
    else {
      *p++ = c;
      c = pop(manager);
    }
  }
}
int write_scraps(file, spelling, defs, global_indent, indent_chars,
                   debug_flag, tab_flag, indent_flag, parameters)
     FILE *file;
     char * spelling;
     Scrap_Node *defs;
     int global_indent;
     char *indent_chars;
     char debug_flag;
     char tab_flag;
     char indent_flag;
     Parameters parameters;
{
  /* This is in file @f */
  int indent = 0;
  while (defs) {
    {
      char c;
      Manager reader;
      Parameters local_parameters = 0;
      int line_number = scrap_array(defs->scrap).file_line;
      reader.scrap = scrap_array(defs->scrap).slab;
      reader.index = 0;
      if (debug_flag) {
        fprintf(file, "\n#line %d \"%s\"\n",
                line_number, scrap_array(defs->scrap).file_name);
        {
          char c1 = pop(&reader);
          char c2 = pop(&reader);

          if (indent_flag && !(c1 == '\n'
                               || c1 == nw_char && (c2 == '#' || (delayed_indent += (c2 == '<'))))) {
            if (tab_flag)
              for (indent=0; indent<global_indent; indent++)
                putc(' ', file);
            else
              for (indent=0; indent<global_indent; indent++)
                putc(indent_chars[indent], file);
          }
          indent = 0;
          backup(2, &reader);
        }
      }
      if (delayed_indent)
      {
        {
          char c1 = pop(&reader);
          char c2 = pop(&reader);

          if (indent_flag && !(c1 == '\n'
                               || c1 == nw_char && (c2 == '#' || (delayed_indent += (c2 == '<'))))) {
            if (tab_flag)
              for (indent=0; indent<global_indent; indent++)
                putc(' ', file);
            else
              for (indent=0; indent<global_indent; indent++)
                putc(indent_chars[indent], file);
          }
          indent = 0;
          backup(2, &reader);
        }
        delayed_indent--;
      }
      c = pop(&reader);
      while (c) {
        switch (c) {
          case '\n': putc(c, file);
                     line_number++;
                     {
                       char c1 = pop(&reader);
                       char c2 = pop(&reader);

                       if (indent_flag && !(c1 == '\n'
                                            || c1 == nw_char && (c2 == '#' || (delayed_indent += (c2 == '<'))))) {
                         if (tab_flag)
                           for (indent=0; indent<global_indent; indent++)
                             putc(' ', file);
                         else
                           for (indent=0; indent<global_indent; indent++)
                             putc(indent_chars[indent], file);
                       }
                       indent = 0;
                       backup(2, &reader);
                     }
                     break;
          case '\t': {
                       if (tab_flag)
                         {
                           int delta = 3 - (indent % 3);
                           indent += delta;
                           while (delta > 0) {
                             putc(' ', file);
                             delta--;
                           }
                         }
                       else {
                         putc('\t', file);
                         if (global_indent + indent < MAX_INDENT) {
                             indent_chars[global_indent + indent] = '\t';
                             indent++;
                         }
                       }
                     }
                     break;
          default:
             if (c==nw_char)
               {
                 {
                   c = pop(&reader);
                   switch (c) {
                     case 'f': fputs(spelling, file);
                               
                               break;
                     case 'x': {
                                  char  label_name[100];
                                  char * p = label_name;
                                  while (c = pop(&reader), c != nw_char) /* Here is @xlabel@x */
                                     *p++ = c;
                                  *p = '\0';
                                  c = pop(&reader);
                                  
                                  write_label(label_name, file);
                               }
                     case '_': break;
                     case '<': {
                                 Name *name = pop_scrap_name(&reader, &local_parameters);
                                 if (local_parameters) {
                                   local_parameters->parent = parameters;
                                 }
                                 if (name->mark) {
                                   fprintf(stderr, "%s: recursive macro discovered involving <%s>\n",
                                           command_name, name->spelling);
                                   exit(-1);
                                 }
                                 if (name->defs) {
                                   name->mark = TRUE;
                                   indent = write_scraps(file, spelling, name->defs, global_indent + indent,
                                                         indent_chars, debug_flag, tab_flag, indent_flag, 
                                                         local_parameters);
                                   indent -= global_indent;
                                   name->mark = FALSE;
                                 }
                                 else
                                 {
                                   if (undefined_flag) {    
                                     int ln = fprintf(file, "%c<%s%c>",  nw_char, name->spelling, nw_char);
                                     for (; --ln >= 0;)
                                     {
                                       indent_chars[global_indent + indent] = ' ';
                                       indent++;
                                     }
                                   }
                                   if (!tex_flag)
                                   fprintf(stderr, "%s: macro never defined <%s>\n",
                                           command_name, name->spelling);
                                 }
                               }
                               if (debug_flag) {
                                 fprintf(file, "\n#line %d \"%s\"\n",
                                         line_number, scrap_array(defs->scrap).file_name);
                                 {
                                   char c1 = pop(&reader);
                                   char c2 = pop(&reader);

                                   if (indent_flag && !(c1 == '\n'
                                                        || c1 == nw_char && (c2 == '#' || (delayed_indent += (c2 == '<'))))) {
                                     if (tab_flag)
                                       for (indent=0; indent<global_indent; indent++)
                                         putc(' ', file);
                                     else
                                       for (indent=0; indent<global_indent; indent++)
                                         putc(indent_chars[indent], file);
                                   }
                                   indent = 0;
                                   backup(2, &reader);
                                 }
                               }
                               break;
                     
                         case '1': case '2': case '3':
                         case '4': case '5': case '6':
                         case '7': case '8': case '9':
                                   if ( parameters && parameters->p[c - '1'] ) {
                                     Scrap_Node param_defs;
                                     param_defs.scrap = parameters->p[c - '1'];
                                     param_defs.next = 0;
                                     write_scraps(file, spelling, &param_defs, global_indent + indent,
                                               indent_chars, debug_flag, tab_flag, indent_flag, 
                                                     parameters? parameters->parent : 0);
                                   } else {
                                     /* ZZZ need error message here */
                                   }
                                   break;
                     
                     default:  
                           if(c==nw_char)
                             {
                               putc(c, file);
                               if (global_indent + indent < MAX_INDENT) {
                                  indent_chars[global_indent + indent] = ' ';
                                  indent++;
                               }
                               break;
                             }
                           /* ignore, since we should already have a warning */
                               break;
                   }
                 }
                 break;
               }         
              putc(c, file);
              if (global_indent + indent < MAX_INDENT) {
                 indent_chars[global_indent + indent] = ' ';
                 indent++;
              }
              break;
        }
        c = pop(&reader);
      }
    }
    defs = defs->next;
  }
  return indent + global_indent;
}
void collect_numbers(aux_name)
     char *aux_name;
{
  if (number_flag) {
    int i;
    for (i=1; i<scraps; i++)
      scrap_array(i).page = i;
  }
  else {
    FILE *aux_file = fopen(aux_name, "r");
    already_warned = FALSE;
    if (aux_file) {
      char aux_line[500];
      while (fgets(aux_line, 500, aux_file)) {
        int scrap_number;
        int page_number;
        char dummy[50];
        if (3 == sscanf(aux_line, "\\newlabel{scrap%d}{%[^}]}{%d}",
                        &scrap_number, dummy, &page_number)) {
          if (scrap_number < scraps)
            scrap_array(scrap_number).page = page_number;
          else
            {
              if (!already_warned) {
                fprintf(stderr, "%s: you'll need to rerun nuweb after running latex\n",
                        command_name);
                already_warned = TRUE;
              }
            }
        }
      }
      fclose(aux_file);
      {
        int scrap;
        for (scrap=2; scrap<scraps; scrap++) {
          if (scrap_array(scrap-1).page == scrap_array(scrap).page) {
            if (!scrap_array(scrap-1).letter)
              scrap_array(scrap-1).letter = 'a';
            scrap_array(scrap).letter = scrap_array(scrap-1).letter + 1;
          }
        }
      }
    }
  }
}
typedef struct name_node {
  struct name_node *next;
  Name *name;
} Name_Node;
typedef struct goto_node {
  Name_Node *output;            /* list of words ending in this state */
  struct move_node *moves;      /* list of possible moves */
  struct goto_node *fail;       /* and where to go when no move fits */
  struct goto_node *next;       /* next goto node with same depth */
} Goto_Node;
typedef struct move_node {
  struct move_node *next;
  Goto_Node *state;
  char c;
} Move_Node;
static Goto_Node *root[128];
static int max_depth;
static Goto_Node **depths;
static Goto_Node *goto_lookup(c, g)
     char c;
     Goto_Node *g;
{
  Move_Node *m = g->moves;
  while (m && m->c != c)
    m = m->next;
  if (m)
    return m->state;
  else
    return NULL;
}
static void build_gotos();
static int reject_match();
static void add_uses();

void search()
{
  int i;
  for (i=0; i<128; i++)
    root[i] = NULL;
  max_depth = 10;
  depths = (Goto_Node **) arena_getmem(max_depth * sizeof(Goto_Node *));
  for (i=0; i<max_depth; i++)
    depths[i] = NULL;
  build_gotos(user_names);
  {
    int depth;
    for (depth=1; depth<max_depth; depth++) {
      Goto_Node *r = depths[depth];
      while (r) {
        Move_Node *m = r->moves;
        while (m) {
          char a = m->c;
          Goto_Node *s = m->state;
          Goto_Node *state = r->fail;
          while (state && !goto_lookup(a, state))
            state = state->fail;
          if (state)
            s->fail = goto_lookup(a, state);
          else
            s->fail = root[a];
          if (s->fail) {
            Name_Node *p = s->fail->output;
            while (p) {
              Name_Node *q = (Name_Node *) arena_getmem(sizeof(Name_Node));
              q->name = p->name;
              q->next = s->output;
              s->output = q;
              p = p->next;
            }
          }
          m = m->next;
        }
        r = r->next;
      }
    }
  }
  {
    for (i=1; i<scraps; i++) {
      signed char c, last = '\0';
      Manager reader;
      Goto_Node *state = NULL;
      reader.prev = NULL;
      reader.scrap = scrap_array(i).slab;
      reader.index = 0;
      c = pop(&reader);
      while (c) {
        while (state && !goto_lookup(c, state))
          state = state->fail;
        if (state)
          state = goto_lookup(c, state);
        else
          state = root[c];
        if (last == nw_char && c == '<')
        {
           int n = 1;
           c = pop(&reader);
           do {
              last = c;
              c = pop(&reader);
              if (last == nw_char)
              {
                 if (c == '>')
                    n--;
                 else if (c == '<')
                    n++;
              }
           }while (n > 0);
        }
        last = c;
        c = pop(&reader);
        if (state && state->output) {
          Name_Node *p = state->output;
          do {
            Name *name = p->name;
            if (!reject_match(name, c, &reader) &&
                scrap_array(i).sector == name->sector &&
                (!name->uses || name->uses->scrap != i)) {
              Scrap_Node *new_use =
                  (Scrap_Node *) arena_getmem(sizeof(Scrap_Node));
              new_use->scrap = i;
              new_use->next = name->uses;
              name->uses = new_use;
              if (!scrap_is_in(name->defs, i))
                add_uses(&(scrap_array(i).uses), name);
            }
            p = p->next;
          } while (p);
        }
      }
    }
  }
}
static void build_gotos(tree)
     Name *tree;
{
  while (tree) {
    {
      int depth = 2;
      char *p = tree->spelling;
      char c = *p++;
      Goto_Node *q = root[c];
      Name_Node * last;
      if (!q) {
        q = (Goto_Node *) arena_getmem(sizeof(Goto_Node));
        root[c] = q;
        q->moves = NULL;
        q->fail = NULL;
        q->moves = NULL;
        q->output = NULL;
        q->next = depths[1];
        depths[1] = q;
      }
      while (c = *p++) {
        Goto_Node *new = goto_lookup(c, q);
        if (!new) {
          Move_Node *new_move = (Move_Node *) arena_getmem(sizeof(Move_Node));
          new = (Goto_Node *) arena_getmem(sizeof(Goto_Node));
          new->moves = NULL;
          new->fail = NULL;
          new->moves = NULL;
          new->output = NULL;
          new_move->state = new;
          new_move->c = c;
          new_move->next = q->moves;
          q->moves = new_move;
          if (depth == max_depth) {
            int i;
            Goto_Node **new_depths =
                (Goto_Node **) arena_getmem(2*depth*sizeof(Goto_Node *));
            max_depth = 2 * depth;
            for (i=0; i<depth; i++)
              new_depths[i] = depths[i];
            depths = new_depths;
            for (i=depth; i<max_depth; i++)
              depths[i] = NULL;
          }
          new->next = depths[depth];
          depths[depth] = new;
        }
        q = new;
        depth++;
      }
      last = q->output;
      q->output = (Name_Node *) arena_getmem(sizeof(Name_Node));
      q->output->next = last;
      q->output->name = tree;
    }
    build_gotos(tree->rlink);
    tree = tree->llink;
  }
}

static int scrap_is_in(Scrap_Node * list, int i)
{
  while (list != NULL) {
    if (list->scrap == i)
      return TRUE;
    list = list->next;
  }
  return FALSE;
}

static void add_uses(Uses * * root, Name *name)
{
   int cmp;
   Uses *p, **q = root;

   while ((p = *q, p != NULL)
          && (cmp = robs_strcmp(p->defn->spelling, name->spelling)) < 0)
      q = &(p->next);
   if (p == NULL || cmp > 0)
   {
      Uses *new = arena_getmem(sizeof(Uses));
      new->next = p;
      new->defn = name;
      *q = new;
   }
}

void
format_uses_refs(FILE * tex_file, int scrap)
{
  Uses * p = scrap_array(scrap).uses;
  if (p != NULL)
    {
      char join = ' ';
      fputs("\\vspace{-2ex}\n", tex_file);
      fputs("\\footnotesize\\addtolength{\\baselineskip}{-1ex}\n", tex_file);
      fputs("\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}", tex_file);
      fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
      fputs("\\item \\NWtxtIdentsUsed\\nobreak\\", tex_file);
      do {
        Name * name = p->defn;
        Scrap_Node *defs = name->defs;
        int first = TRUE, page = -1;
        fprintf(tex_file,
                "%c \\verb%c%s%c\\nobreak\\ ",
                join, nw_char, name->spelling, nw_char);
        do {
          fputs("\\NWlink{nuweb", tex_file);
          write_scrap_ref(tex_file, defs->scrap, -1, &page);
          fputs("}{", tex_file);
          write_scrap_ref(tex_file, defs->scrap, first, &page);
          fputs("}", tex_file);
          first = FALSE;
          defs = defs->next;
        }while (defs!= NULL);
        
        join = ',';
        p = p->next;
      }while (p != NULL);
      fputs(".", tex_file);
      fputs("\\end{list}\n", tex_file);
    }
}

void
format_defs_refs(FILE * tex_file, int scrap)
{
  Uses * p = scrap_array(scrap).defs;
  if (p != NULL)
    {
      char join = ' ';
      fputs("\\vspace{-2ex}\n", tex_file);
      fputs("\\footnotesize\\addtolength{\\baselineskip}{-1ex}\n", tex_file);
      fputs("\\begin{list}{}{\\setlength{\\itemsep}{-\\parsep}", tex_file);
      fputs("\\setlength{\\itemindent}{-\\leftmargin}}\n", tex_file);
      fputs("\\item \\NWtxtIdentsDefed\\nobreak\\", tex_file);
      do {
        Name * name = p->defn;
        Scrap_Node *defs = name->uses;
        int first = TRUE, page = -1;
        fprintf(tex_file,
                "%c \\verb%c%s%c\\nobreak\\ ",
                join, nw_char, name->spelling, nw_char);
        if (defs == NULL
            || (defs->scrap == scrap && defs->next == NULL)) {
          fputs("\\NWtxtIdentsNotUsed", tex_file);
        }
        else {
          do {
            if (defs->scrap != scrap) {
               fputs("\\NWlink{nuweb", tex_file);
               write_scrap_ref(tex_file, defs->scrap, -1, &page);
               fputs("}{", tex_file);
               write_scrap_ref(tex_file, defs->scrap, first, &page);
               fputs("}", tex_file);
               first = FALSE;
            }
            defs = defs->next;
          }while (defs!= NULL);
        }
        
        join = ',';
        p = p->next;
      }while (p != NULL);
      fputs(".", tex_file);
      fputs("\\end{list}\n", tex_file);
    }
}
#define sym_char(c) (isalnum(c) || (c) == '_')

static int op_char(c)
     char c;
{
  switch (c) {
    case '!':           case '#': case '%': case '$': case '^': 
    case '&': case '*': case '-': case '+': case '=': case '/':
    case '|': case '~': case '<': case '>':
      return TRUE;
    default:
      return c==nw_char ? TRUE : FALSE;
  }
}
static int reject_match(name, post, reader)
     Name *name;
     char post;
     Manager *reader;
{
  int len = strlen(name->spelling);
  char first = name->spelling[0];
  char last = name->spelling[len - 1];
  char prev = '\0';
  len = reader->index - len - 2;
  if (len >= 0)
    prev = reader->scrap->chars[len];
  else if (reader->prev)
    prev = reader->scrap->chars[SLAB_SIZE - len];
  if (sym_char(last) && sym_char(post)) return TRUE;
  if (sym_char(first) && sym_char(prev)) return TRUE;
  if (op_char(last) && op_char(post)) return TRUE;
  if (op_char(first) && op_char(prev)) return TRUE;
  return FALSE; /* Here is @xother@x */
}
void
write_label(char label_name[], FILE * file)
{
   label_node * * plbl = &label_tab;
   for (;;)
   {
      label_node * lbl = *plbl;

      if (lbl)
      {
         int cmp = label_name[0] - lbl->name[0];

         if (cmp == 0)
            cmp = strcmp(label_name + 1, lbl->name + 1);
         if (cmp < 0)
            plbl = &lbl->left;
         else if (cmp > 0)
            plbl = &lbl->right;
         else
         {
            write_single_scrap_ref(file, lbl->scrap);
            fprintf(file, "-%02d", lbl->seq);
            break;
         }
      }
      else
      {
          fprintf(stderr, "Can't find label %s.\n", label_name);
          break;
      }
   }
}

