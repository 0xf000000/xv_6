#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

//Define max line length and max number of rows
#define MAX_LINE_LENGTH 1000
#define MAX_Rows 14

// struct to have item( text, count of duplicates)
struct Item
{
    char text[MAX_Rows];
    int count_dup;
};

//strncpy implementation
char *
strncpy(char *s, const char *t, int n)
{
    char *value;

    value = s;


    //copies n characters from t to s
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;
    // pad remaining characters in s with null characters
    while (n-- > 0)
        *s++ = 0;
    return value; // returns to the beginning of s
}

//main
int main(int argc, char *argv[])
{
    //file descriptor
    int fd = 0;
    const char *path;
    char hidden[MAX_LINE_LENGTH];// holds input data
   
    if (argc >= 1)
    {
        for (int inputIndex = 0; inputIndex <= argc; inputIndex++)
        {
           
            if ((strlen(argv[inputIndex]) > 4) || (argc == 1) || (argc == 2 && ((strcmp(argv[1], "-c") == 0) || (strcmp(argv[1], "-u") == 0) || (strcmp(argv[1], "-w") == 0))))
            {
                path = argv[inputIndex]; // getting path
                //opening file
                fd = open(path, O_RDONLY);
                if (fd < 0)
                {
                    printf(1, "Error: Cannot open file %s\n", path);
                   exit();
                 }
              
                int Readhidden__ = -1;
                //reads input data from file based on arguments
                if (argc == 1)
                {
                    Readhidden__ = read(0, hidden, sizeof(hidden) - 1);
                }
                else if (argc == 2 && ((strcmp(argv[1], "-c") == 0) || (strcmp(argv[1], "-u") == 0) || (strcmp(argv[1], "-w") == 0)))
                {
                    Readhidden__ = read(0, hidden, sizeof(hidden) - 1);
                }
                else if (argc == 3 && ((strcmp(argv[1], "-w") == 0)))
                {
                    Readhidden__ = read(0, hidden, sizeof(hidden) - 1);
                }
                else
                {
                    Readhidden__ = read(fd, hidden, sizeof(hidden) - 1);
                }
                
                hidden[Readhidden__] = '\0';
                //checking if last character is a newline
                char lastone[2]; // +1 for the null terminator
                strncpy(lastone, hidden + Readhidden__ - 1, 1);
                if (lastone[0] != '\n')
                {
                    hidden[Readhidden__] = '\n';
                    hidden[Readhidden__ + 1] = '\0';
                    
                    strncpy(lastone, hidden + Readhidden__ - 1, 1);
             
                }
                //initialising variables to store current and prev lines of data
                char Curr_linehidden[512] = "";
                char Prev_linehidden[512] = "";
                char Orr_linehidden[512] = "";
                char hidden_original_pre[512] = "";
                int current_line_index = 0, count_dup = 1;
              
                int count_w_0=0;
                //runs for each character in the input data
                for (int i = 0; i <= Readhidden__; i++)
                {
                    if (hidden[i] == '\n')
                    { 
                        count_w_0++;
                        if (strcmp(argv[1], "-w") == 0){
                            int letter_compare_count = atoi(argv[2]);
                            char hidden_le_cur[letter_compare_count + 1]; // +1 for null terminator
                            char hidden_le_pre[letter_compare_count + 1]; // +1 for null terminator
                            
                            //To Copy first letter_compare_count characters from current and previous lines
                            strncpy(hidden_le_cur, Curr_linehidden, letter_compare_count);
                            strncpy(hidden_le_pre, Prev_linehidden, letter_compare_count);
                            
                            // Null-terminate the strings
                            hidden_le_cur[letter_compare_count] = '\0';
                            hidden_le_pre[letter_compare_count] = '\0';

                            // Comparing truncated strings
                            if (letter_compare_count==0 ){
                                if (count_w_0==1){
                                printf(1,"%s\n",Curr_linehidden);
                            }}
                            else if (strcmp(hidden_le_cur, hidden_le_pre) != 0) {
                                printf(1, "%s\n", Curr_linehidden);
                                strcpy(Prev_linehidden, Curr_linehidden);
                                strcpy(hidden_original_pre, Orr_linehidden);
                            }
                        }
                        else{
                            //increase count if current line is the same as previous line
                        if (strcmp(Curr_linehidden, Prev_linehidden) == 0)
                        {
                            count_dup = count_dup + 1;
                        }
                        else
                        {
                            //prints prevline and its duplicate count
                            if (strlen(Prev_linehidden) > 0)
                            {
                                if (strcmp(argv[1], "-c") == 0)
                                {
                                    printf(1,"\t%d %s\n", count_dup, Prev_linehidden);
                                }
                                else if (strcmp(argv[1], "-u") == 0) 
                                {
                                    if (count_dup==1){
                                    printf(1,"%s\n", Prev_linehidden);}
                                }
                                
                                else
                                {
                                    printf(1,"%s\n", Prev_linehidden);
                                }
                                count_dup = 1;
                            }
                            //coping current line to prev line
                            strcpy(Prev_linehidden, Curr_linehidden);
                            strcpy(hidden_original_pre, Orr_linehidden);

                            count_dup = 1;
                        }}
                        //clears current line
                        memset(Curr_linehidden, 0, sizeof(Curr_linehidden));
                        memset(Orr_linehidden, 0, sizeof(Orr_linehidden));
                        current_line_index = 0;
                    }
                    else
                    {
                        //copies data to current line bufer
                        Curr_linehidden[current_line_index] = hidden[i];
                        Orr_linehidden[current_line_index] = hidden[i];
                        current_line_index++;
                    }
                }
                //To print last line
                //-c flag
                if (strcmp(argv[1], "-c") == 0)
                {
                    printf(1,"\t%d %s\n", count_dup, Prev_linehidden);
                }
                //-u flag
                else if (strcmp(argv[1], "-u") == 0) 
                {
                    if (count_dup==1){
                        printf(1,"%s\n", Prev_linehidden);}
                }
               //-w flag
                else if (strcmp(argv[1], "-w") != 0) 
                {
                    printf(1,"%s\n", Prev_linehidden);
                }

                exit();
            }
            // close file descriptor
            close(fd);
        }
    }
    else{

        //to print invalid arguments
        printf(1,"Invalid Arguments");
    }
    //exits program
    exit();
}