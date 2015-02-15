/*-----------------------------------------------------------------------------*/
/* TSort - Text File Sorting Program                                           */
/* Version: 1.0                                                                */
/* Author:  Trevor Barnett                                                     */
/* Date:    15/02/99                                                           */
/* Copyright Trevor Barnett 1999                                               */
/*-----------------------------------------------------------------------------*/

/*----------------------------- Includes --------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*------------------------ Constant Definitions -------------------------------*/
#define STORE_LENGTH 255
#define EOF_CODE     STORE_LENGTH+1
#define ERROR_CODE   STORE_LENGTH+2
#define TRUE         1
#define FALSE        0
#define CASE_DIFF    'a'-'A'
#define COMP_ERROR   10  /* ie a number >1 */

/*------------------------- Struct Definitions --------------------------------*/
struct line{
   struct line *next_line;
   char        *text;
};

struct block{
   struct block *next_block;
   struct block *prev_block;
   struct line  *first_line;
};

/*------------- Initialise global variables and defaults ----------------------*/
int g_lines_per_block = 1;
int g_sort_column     = 0;
int g_sort_line       = 0;
int g_reverse         = FALSE;
int g_insensitive     = FALSE;
char *g_infile        = NULL;
char *g_outfile       = NULL;
struct block *g_first_block = NULL;
struct block *g_last_block  = NULL;

/*------------------------------------------------------------------------------*/
/* test reading in of lines                                                     */
int main(int argc, char **argv)
{ 
   FILE *fptr;
   struct block *block_ptr;

   void getargs(int argc, char **argv);    
   struct block *read_block(FILE *fptr);
   void freememory(void);
   void freelines(struct block *block_ptr);
   int insert_block(struct block *block_ptr);
   void put_blocks(FILE *fptr);  
   void print_about(void); 

   print_about();
   getargs(argc,argv); /* read in command line args */
     
   fptr=fopen(g_infile,"r");
   if (fptr==NULL)
   {
       printf("Cannot open input file.\n");
       exit(10);
   }
   
   printf("\nSorting file....\n");

   do {
       block_ptr = read_block(fptr);
       if(block_ptr==NULL)
       {
           /* either not enough memory or nothing in the file to sort */
           printf("Error: Allocation failure.\n");
           /* some blocks of memory may have been allocated */
           freememory();
           exit(10);
       }
       if(block_ptr==g_first_block) break;
       if(insert_block(block_ptr)==COMP_ERROR)
       {
           freelines(block_ptr);
           free(block_ptr);
           freememory();
           fclose(fptr);
           printf("Cannot allocate memory.\n");
           exit(10);
       }
   }while (TRUE);  
   fclose(fptr);
   
   fptr = fopen(g_outfile, "w");
   if (fptr==NULL)
   {
       printf("Cannot open output file.\n");
       freememory();
       exit(10);
   }
   put_blocks(fptr);
   fclose(fptr);
   freememory();
   printf("Operation completed successfully.\n");
   return(0);
}
/*------------------------------------------------------------------------------*/
void getargs(int argc, char **argv)
{
   int i, num;
   void showargs(void);
   
   for(i=1; i<argc; i++)
   {
       if(*argv[i]=='?') showargs();
       if(*argv[i]=='-')
       {
           switch(argv[i][1])
           {
               case 'b':
                   num=atoi(&(argv[i][2]));
                   if (num>0) g_lines_per_block = num;
                  break;
               case 'c':
                   num=atoi(&(argv[i][2]));
                  if (num>0) g_sort_column = num-1;
                   break;
               case 'l':
                   num=atoi(&(argv[i][2]));
                   if (num>0) g_sort_line = num-1;
                   break;
               case 'f':
                   g_reverse = FALSE;
                   break;
               case 'r':
                   g_reverse = TRUE;
                   break;
               case 'i':
                   g_insensitive = TRUE;
                   break;
               case 's':
                   g_insensitive = FALSE;
                   break;
               case '?':
                   showargs();
                   break;
           } /* end switch */
       }
       else /* if *argv!='-' */
       {
           if (g_infile == NULL)
           {
               g_infile=argv[i];
           }
           else
           {
               if(g_outfile == NULL)
               {
                   g_outfile = argv[i];
               }
           }
       } /* end if-else */
   } /* end loop over args */
   if (g_infile==NULL || g_outfile==NULL)
   {
       if(g_infile==NULL)
       {
           printf("Input file required.\n");
       }
       if(g_outfile==NULL)
       {
           printf("Output file required.\n");
       }
       showargs();
   }
   if (g_sort_line>=g_lines_per_block) g_sort_line=g_lines_per_block-1;
} /* end getargs() */
/*-----------------------------------------------------------------------------*/

void showargs(void)
{
   printf("\nOptions are:\n");
   printf("\t[-b<lines_per_block>]         default=1\n");
   printf("\t[-c<sort_start_column>]       default=1\n");
   printf("\t[-l<sort_start_line>]         default=1\n");
   printf("\t[-f] - forward ascii sort     default\n");
   printf("\t[-r] - reverse ascii sort\n");
   printf("\t[-s] - case-sensitive sort    default\n");
   printf("\t[-i] - case-insensitive sort\n");
   printf("\t<input file name>\n");
   printf("\t<output file name>\n\n");
   exit(0);
}   

/*-----------------------------------------------------------------------------*/
/* Display author and version informations                                     */
void print_about(void)
{
   printf("\nTSort - text file sort program\n");
   printf("Version 1.0 - 24/02/99\n");
   printf("Copyright Trevor Barnett 1999\n");
}

/*------------------------------------------------------------------------------*/
int insert_block(struct block *block_ptr)
{
   struct block *crnt_block, *prev_block;
   int comp_value;
   int compare(const struct block *block1, const struct block *block2);
   
   if (g_first_block==NULL)
   {
       g_first_block = block_ptr;
       g_last_block  = block_ptr;
       return(0); /* everything okay */
   }
   crnt_block=g_first_block;
   comp_value = compare(block_ptr, crnt_block);
   if(comp_value==COMP_ERROR) return(COMP_ERROR);
   while(comp_value>0)
   { /* loop while "block_ptr>crnt_block" */
       prev_block = crnt_block;
       crnt_block = prev_block->next_block;
       if(crnt_block==NULL) break;
       comp_value=compare(block_ptr, crnt_block);
       if(comp_value==COMP_ERROR) return (COMP_ERROR);
   }
   if (crnt_block==NULL) /* last block in list */
   {
       g_last_block = block_ptr;
       prev_block->next_block = g_last_block;
       g_last_block->prev_block = prev_block;
       g_last_block->next_block = NULL;
       return(0); /* everything okay */
   }
   if (crnt_block == g_first_block) /* first block in list */
   {
       g_first_block->prev_block = block_ptr;
       block_ptr->next_block = g_first_block;
       g_first_block = block_ptr;
       g_first_block->prev_block = NULL;
       return(0); /* everything okay */
   }
   /* block is somewhere in the middle of the list */
   prev_block->next_block=block_ptr;
   crnt_block->prev_block=block_ptr;
   block_ptr->next_block=crnt_block;
   block_ptr->prev_block=prev_block;
   return(0); /* everything okay */
}
/*------------------------------------------------------------------------------*/
/* compare strings at line g_sort_line and column g_sort_column of two blocks   */
/* return values same as for strcmp()                                           */
int compare(const struct block *block1, const struct block *block2)
{
   struct line *line1_p, *line2_p;
   char *cmpstr1_p, *cmpstr2_p, *cmpstr3_p;
   int line_cnt, size, comp_value;
   
   char *str_upper(char *str);
   
   line_cnt = g_sort_line;
   line1_p = block1->first_line;
   line2_p = block2->first_line;
   while(line_cnt)
   {
       /* This block only entered if sort line is not the first line, since line*/
       /* one is zero 'internally', so line_cnt=0, ie false.                    */
       line1_p=line1_p->next_line;
       line2_p=line2_p->next_line;
       line_cnt--;
   } /* step through list `till correct line is reached */
   /* get strings to be compared - stored in text field of line structure */
   cmpstr1_p=&(line1_p->text[g_sort_column]);
   cmpstr2_p=&(line2_p->text[g_sort_column]);
   if (g_insensitive)
   {
       size = strlen(cmpstr1_p);
       cmpstr3_p = (char *)malloc(size+1);
       if (cmpstr3_p == NULL)
       {
           return(COMP_ERROR);
       }
       (void)strcpy(cmpstr3_p, cmpstr1_p);
       cmpstr1_p=str_upper(cmpstr3_p);
       size = strlen(cmpstr2_p);
       cmpstr3_p = (char *)malloc(size+1);
       if (cmpstr3_p == NULL)
       {
           free(cmpstr1_p);
           return(COMP_ERROR);
       }
       (void)strcpy(cmpstr3_p, cmpstr2_p);
       cmpstr2_p=str_upper(cmpstr3_p);
   }
   
   if (g_reverse)
   {
       comp_value = strcmp(cmpstr2_p, cmpstr1_p);
   }
   else
   {
       comp_value = strcmp(cmpstr1_p, cmpstr2_p);
   }
   if(g_insensitive)
   {
       free(cmpstr1_p);
       free(cmpstr2_p);
   }
   if (comp_value>0) return (1);
   if (comp_value<0) return (-1);
   if (comp_value==0) return (0);
   return (COMP_ERROR); /* should never ever reach here!! */
} /* end compare() */

/*-------------------------------------------------------------------------------*/
/* Convert string to uppercase characters.  Returns pointer to modified string   */
char *str_upper(char *str)
{
   int i;
   
   for(i=0; str[i]!='\0'; i++)
   {
       if(str[i]>='a' && str[i]<='z')
       {
           str[i]-=CASE_DIFF;
       }
   }
   return(str);
} /* end str_upper() */

/*-------------------------------------------------------------------------------*/
/* read_block() reads text into buffer called text_store 255 bytes long.         */
/* If whole line won`t fit buffer transferred to a dynamically allocated string  */
/* and more text is read.  Repeat procedure until line ends.                     */
/* Lines are padded to size of g_sort_column and extra lines are added to ensure */
/* comparison function works correctly.                                          */

struct block *read_block(FILE *fptr)
{
   struct block *block_ptr;
   struct line  *line_ptr, *crnt_line;
   char         *str_ptr, *new_str_ptr, text_store[STORE_LENGTH];
   int          read_length, line_length, line_cnt, i;
   
   void            freelines(struct block *block_ptr);
   struct block   *allocblock(void);
   struct line    *allocline(void);
   char           *allocstring(int length);
   int             getline(char *str, FILE *fptr);
   
   block_ptr = allocblock();
   if (block_ptr==NULL) return (NULL);
   block_ptr->next_block = NULL;
   block_ptr->prev_block = NULL;
   block_ptr->first_line = NULL;
   crnt_line = NULL;
   line_cnt = 0;
   
   do { /* while line_cnt<g_lines_per_block, ie while more lines left to read */
       line_length = 0;
       str_ptr = NULL;
       do { /* while whole line not read in */
           read_length = getline(text_store, fptr);
           if (read_length==EOF_CODE || read_length==ERROR_CODE)
           { /* break out of loop if end of file or read error occurs */
               break;
           }
           line_length = line_length + read_length;
           new_str_ptr = allocstring(line_length);
           if (new_str_ptr==NULL)
           {
               free(block_ptr);
               if (str_ptr!=NULL) free(str_ptr);
               return(NULL);
           }
           if (str_ptr!=NULL)
           {
               /* Have already read in part of line, whole line will not fit in */
               /* buffer.  Copy stored part of line into new string and join onto */
               /* new section of line just read into text_store with getline() */
               strcpy(new_str_ptr, str_ptr);
               strcat(new_str_ptr, text_store);
               free(str_ptr); /* old string no longer needed */
           }
           else
           {
               /* need to copy text_store into allocated string!!! */
               strcpy(new_str_ptr, text_store);
           }
           str_ptr = new_str_ptr;  /* change name ready to loop */
       } while (read_length == STORE_LENGTH-1);
       /* need STORE_LENGTH-1 since last byte of string is '\0' */
       if (read_length==EOF_CODE) /* broke out of loop above */
       {
           /* need to break out even if already have a line so that padding works */
           break;
       }
       if (read_length==ERROR_CODE)
       {
           break;
       }
       /* pad strings with '\0' if necessary to ensure compare function works correctly */
       if (line_length<g_sort_column)
       {

                      new_str_ptr = allocstring(g_sort_column);
           if (new_str_ptr == NULL)
           {
               freelines(block_ptr);
               free(str_ptr);
               free(block_ptr);
               return (NULL);
           }
           strcpy(new_str_ptr, str_ptr);
           for(i=line_length; i<g_sort_column; i++)
           {
               new_str_ptr[i]='\0';
           }
           free(str_ptr); /* don't need old string */
           str_ptr = new_str_ptr;  /* "rename" so everything matches */
       }
       line_ptr = allocline();
       if (line_ptr==NULL)
       {
           freelines(block_ptr);
           free(str_ptr);
           free(block_ptr);
           return (NULL);
       }
       line_ptr->text = str_ptr;
       if (block_ptr->first_line == NULL)
       {
           block_ptr->first_line = line_ptr;
       }
       else
       {
           crnt_line->next_line = line_ptr;
       }
       crnt_line=line_ptr;
       crnt_line->next_line=NULL;
       if (read_length!=EOF_CODE) line_cnt++;
   }while((line_cnt<g_lines_per_block) && (read_length !=EOF_CODE));
   
   if (read_length==EOF_CODE)
   {
       if(line_cnt==0)
       {
           free(block_ptr); /* block is empty so free it, no other memory allocated */
           return (g_first_block); /* signal all blocks read in */
       }
       /* pad out with extra lines so compare works correctly.  There should be by  */
       /* this point at least one line  */                                         
       while(line_cnt<g_lines_per_block)
       {
           line_ptr=allocline();
           if (line_ptr==NULL)
           {
               freelines(block_ptr);
               free(block_ptr);
               return (NULL);
           }
           str_ptr=allocstring(g_sort_column);
           if(str_ptr==NULL)
           {
               free(line_ptr);
               freelines(block_ptr);
               free(block_ptr);
               return (NULL);
           }
           for (i=0; i<=g_sort_column;i++)
           {
               /* set every character in string to '\0' */
               str_ptr[i]='\0';
           }
           crnt_line->next_line=line_ptr;
           crnt_line=line_ptr;
           crnt_line->text=str_ptr;
           crnt_line->next_line=NULL; /* forgot to initialise this !! */
           line_cnt++;
       } /* end while(line_cnt<g_lines_per_block) */
   } /* end if(read_length==EOF_CODE) */
   if (read_length==ERROR_CODE)
   {
       free(block_ptr);
       return (NULL);
   }
   return (block_ptr);
} /* end read_block() */

/*------------------------------------------------------------------------------*/
int getline(char *str, FILE *fptr)
{
   /* read in a line, or part of a line, upto size of buffer, STORE_LENGTH */
   int line_length;
   char *fgetstr(char *str, int n, FILE *fptr);
   if (fgetstr(str,STORE_LENGTH,fptr)==NULL)
   {
       if (feof(fptr))
       {
           return(EOF_CODE);
       }
       else
       {
           return (ERROR_CODE);
       }
   }
   line_length=strlen(str);
   return(line_length);
}
/*------------------------------------------------------------------------------*/
/* fgetstr() modifies library function fgets() in order to remove unwanted      */
/* linefeeds.                                                                   */
char *fgetstr(char *str, int n, FILE *fptr)
{
   int str_length;
   
   if (fgets(str,n,fptr)==NULL) return (NULL);
   str_length = strlen(str);
   if(str_length>0)
   {
       if(str[str_length-1]=='\n')
       {
           str[str_length-1]='\0';
       }
   }
   return (str);
} /* end fgetstr() */


/*-----------------------------------------------------------------------------*/
void freelines(struct block *block_ptr)
{
   struct line *line_ptr, *prev_line_ptr;
       
   line_ptr=block_ptr->first_line;
   while (line_ptr!=NULL)
   {
       prev_line_ptr=line_ptr;
       line_ptr=line_ptr->next_line;
       free(prev_line_ptr->text);
       free(prev_line_ptr);
   }
} /* end freelines() */

/*-----------------------------------------------------------------------------*/
struct block *allocblock(void)
{
   struct block *block_ptr;
   
   block_ptr=(struct block *)malloc(sizeof(struct block));
   return(block_ptr);
}

/*-----------------------------------------------------------------------------*/
struct line *allocline(void)
{
   struct line *line_ptr;
   
   line_ptr=(struct line*)malloc(sizeof(struct line));
   return(line_ptr);
}

/*------------------------------------------------------------------------------*/
char *allocstring(int length)
{
   char *str_ptr;

   str_ptr=(char *)malloc(length+1);
   return(str_ptr);
}

void freememory(void)
{
   struct block *block_ptr, *prev_block_ptr;
   struct line  *line_ptr,  *prev_line_ptr;
  
   block_ptr=g_first_block;
   while(block_ptr!=NULL)
   {
       line_ptr=block_ptr->first_line;
       while(line_ptr!=NULL)
       {
           prev_line_ptr=line_ptr;
           line_ptr=line_ptr->next_line;
           free(prev_line_ptr->text);
           free(prev_line_ptr);
       }
       prev_block_ptr=block_ptr;
       block_ptr=block_ptr->next_block;
       free(prev_block_ptr);
   }
}

/*-----------------------------------------------------------------------------*/
void put_blocks(FILE *fptr)
{
   /* write sorted text to output file */
   struct block *crnt_block;
   struct line  *line_ptr;
   
   crnt_block = g_first_block;
   while(crnt_block!=NULL)
   {
       line_ptr = crnt_block->first_line;
       while(line_ptr!=NULL)
       {
           if(line_ptr->text!=NULL)
           {
               fputs(line_ptr->text, fptr);
               (void)fputc('\n',fptr);
           }
           line_ptr=line_ptr->next_line;
       }
       crnt_block=crnt_block->next_block;
   }
} /* end put_blocks() */
/*-----------------------------------------------------------------------*/
/*------------------------ End of program -------------------------------*/
/*-----------------------------------------------------------------------*/