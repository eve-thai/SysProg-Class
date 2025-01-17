#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void reverse_buffer(char *, int);
void write_buffer(char *, int);


int setup_buff(char *buff, char *user_str, int len){
    int str_len = strlen(user_str);
    if (str_len >= len) {
        memcpy(buff, user_str, len - 1);
        buff[len - 1] = '\0';
        return len - 1;
    } else {
        strcpy(buff, user_str);
        return str_len;
    }
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    int count = 0;
    int in_word = 0;

    for (int i = 0; i < str_len && i < len; i++) {
        if (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\n' || buff[i] == '\0') {
            if (in_word) {
                count++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }

    if (in_word) count++;
    return  count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
void reverse_buffer(char *buff, int len) {
    int start = 0;
    int end = strlen(buff) - 1;

    while (start < end) {
        char temp = buff[start];
        buff[start] = buff[end];
        buff[end] = temp;
        start++;
        end--;
    }
}

void write_buffer(char *buff, int len) {
    printf("Word Print\n");
    printf("----------\n");

    int word_count = 1;
    int start = 0;
    for (int i = 0; i <= strlen(buff); i++) {
        if (buff[i] == ' ' || buff[i] == '\0') {
            if (i > start) {
                printf("%d. ", word_count++);
                for (int j = start; j < i; j++) {
                    putchar(buff[j]);
                }
                printf(" (%d)\n", i - start);
                start = i + 1;
            } else {
                start = i + 1;
            }
        }
    }
}

void replace_string(char *buff, const char *old_word, const char *new_word, int len) {
    char temp[BUFFER_SZ];
    char *pos, *start = buff;
    int old_len = strlen(old_word);
    int new_len = strlen(new_word);
    int temp_index = 0;

    // Clear the temporary buffer
    memset(temp, 0, BUFFER_SZ);

    while ((pos = strstr(start, old_word)) != NULL) {
        // Copy text before the occurrence
        int bytes_to_copy = pos - start;
        if (temp_index + bytes_to_copy >= BUFFER_SZ - 1) break;
        memcpy(temp + temp_index, start, bytes_to_copy);
        temp_index += bytes_to_copy;

        // Copy the new word
        if (temp_index + new_len >= BUFFER_SZ - 1) break;
        memcpy(temp + temp_index, new_word, new_len);
        temp_index += new_len;

        // Move the start pointer
        start = pos + old_len;
    }

    // Copy the remaining part of the string
    if (temp_index + strlen(start) < BUFFER_SZ - 1) {
        strcpy(temp + temp_index, start);
    }

    // Copy back to the original buffer
    strncpy(buff, temp, len - 1);
    buff[len - 1] = '\0';
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
    If argv[1] does not exist, 
    argc < 2 ensures we do not dereference argv[1].
    */   
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
    Ensure at least 3 arguments. argv[2] holds the string.
    Missing argv[2] would result in undefined behavior.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        fprintf(stderr, "Error: Failed to allocate memory for buffer.\n");
        exit(99);
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_buffer(buff, BUFFER_SZ);
            printf("Reversed buffer: \n");
            print_buff(buff, BUFFER_SZ);
            break;

        case 'w':
            write_buffer(buff, BUFFER_SZ);
            break;

        case 'x':
            if (argc < 5) {
                fprintf(stderr, "Error: Missing arguments for string replacement.\n");
                usage(argv[0]);
                free(buff);
                exit(1);
            }
            replace_string(buff, argv[3], argv[4], BUFFER_SZ);
            printf("Modified String: %s\n", buff);
            break;

        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
/*
Providing both ensures,
we avoid overflows and stay within the buffer's allocated memory,
especially if the buffer contains dynamic content or a null terminator might be missing.
*/