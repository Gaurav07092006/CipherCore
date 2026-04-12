#include <winsock2.h> 
#include <windows.h>  
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <time.h>   

#pragma comment(lib, "ws2_32.lib") 

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

extern int encrypt_char(int ch, int shift);
extern int decrypt_char(int ch, int shift);

const char *DB_FILE = "database.txt";
const char *HISTORY_FILE = "history.txt"; 
const char *COLLEGE_DOMAIN = "@nitdelhi.ac.in"; 
const int DB_SHIFT = 7; 

// --- GROUP CHAT GLOBAL VARIABLES ---
#define MAX_CLIENTS 9 
SOCKET client_sockets[MAX_CLIENTS];
char client_aliases[MAX_CLIENTS][50]; 
char client_ips[MAX_CLIENTS][20]; 
volatile int client_count = 0; 
int is_host = 0; 

char chat_history[15][3000];
int history_count = 0;

SOCKET current_socket; 
SOCKET host_listen_socket; 
int chat_shift_key = 0;
int chat_active = 1;
char my_alias[50] = "User";

// --- TYPING, WAITING ROOM & TRANSFER GLOBALS ---
time_t typing_expiry = 0;
char typing_user[50] = {0};
time_t last_sent_typing = 0;
volatile int pending_approval = 0;
volatile int approval_decision = 0; 
volatile int cancel_transfer = 0; 

// ASYNC TYPING BUFFERS
char current_input[1024] = {0};
int input_len = 0;

// GLOBAL MUTEX LOCKS
CRITICAL_SECTION console_mutex; 
CRITICAL_SECTION array_mutex;   

// --- SIMPLE MENU GETCHAR (FORWARD DECLARATION) ---
char menu_getch();

// --- C MIDDLEWARE: Hexadecimal Encoders ---
void string_to_hex(const char* input, char* output) {
    while(*input) {
        sprintf(output, "%02X", (unsigned char)(*input));
        output += 2;
        input++;
    }
    *output = '\0';
}

void hex_to_string(const char* input, char* output) {
    while(*input && *(input+1)) {
        char hex_byte[3] = {input[0], input[1], '\0'};
        unsigned int val;
        sscanf(hex_byte, "%x", &val);
        *output = (char)val;
        output++;
        input += 2;
    }
    *output = '\0';
}

// --- COLOR HASHING ALGORITHM (djb2) ---
int get_color_code(const char *name) {
    unsigned int hash = 5381;
    int c;
    while ((c = *name++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    int colors[] = {32, 33, 34, 35, 36, 92, 93, 94, 95, 96}; 
    return colors[hash % 10];
}

// --- MATH: DIFFIE-HELLMAN ALGORITHM ---
long long int power_mod(long long int base, long long int exp, long long int mod) {
    long long int res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return res;
}

// --- FEATURE 5: ENCRYPTED STEGANOGRAPHY DEMO (DYNAMIC HEADER PATCH) ---
void steganography_demo() {
    system("cls");
    printf("=========================================\n");
    printf("||    ENCRYPTED LSB STEGANOGRAPHY      ||\n");
    printf("=========================================\n\n");
    printf("1. Hide a Secret Message in a BMP Image\n");
    printf("2. Extract a Secret Message from a BMP Image\n");
    printf("3. Return to Main Menu\n\n");
    printf("Enter choice: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) choice = 0;
    int c; while ((c = getchar()) != '\n' && c != EOF);

    if (choice == 1) {
        char src_file[100], dest_file[100], secret_msg[1024];
        int pin;

        printf("\nEnter source BMP filename (e.g., test.bmp): ");
        fgets(src_file, sizeof(src_file), stdin);
        src_file[strcspn(src_file, "\n")] = 0;

        printf("Enter output BMP filename (e.g., secret_test.bmp): ");
        fgets(dest_file, sizeof(dest_file), stdin);
        dest_file[strcspn(dest_file, "\n")] = 0;

        printf("Enter 4-digit Stego-PIN (Encryption Key): ");
        if (scanf("%d", &pin) != 1) pin = 1234;
        while ((c = getchar()) != '\n' && c != EOF);

        printf("Enter the Secret Message to hide: ");
        fgets(secret_msg, sizeof(secret_msg), stdin);
        secret_msg[strcspn(secret_msg, "\n")] = 0;

        FILE *f_in = fopen(src_file, "rb");
        FILE *f_out = fopen(dest_file, "wb");

        if (!f_in || !f_out) {
            printf("\n\x1b[31m[-] Error: Could not open image files. Make sure the BMP file exists!\x1b[0m\n");
            if(f_in) fclose(f_in);
            if(f_out) fclose(f_out);
        } else {
            unsigned char header_info[54];
            fread(header_info, 1, 54, f_in);
            int pixel_offset = header_info[10] | (header_info[11] << 8) | (header_info[12] << 16) | (header_info[13] << 24);
            if (pixel_offset < 54 || pixel_offset > 2000) pixel_offset = 54;
            
            rewind(f_in);
            char *full_header = malloc(pixel_offset);
            fread(full_header, 1, pixel_offset, f_in);
            fwrite(full_header, 1, pixel_offset, f_out);
            free(full_header);

            int msg_len = strlen(secret_msg);
            int xor_key = (pin % 255) + 1;

            for (int i = 0; i < 32; i++) {
                int bit = (msg_len >> i) & 1;
                int byte = fgetc(f_in);
                if (byte == EOF) break;
                byte = (byte & 0xFE) | bit; 
                fputc(byte, f_out);
            }

            for (int i = 0; i < msg_len; i++) {
                char encrypted_char = secret_msg[i] ^ xor_key;
                for (int j = 0; j < 8; j++) {
                    int bit = (encrypted_char >> j) & 1;
                    int byte = fgetc(f_in);
                    if (byte == EOF) break;
                    byte = (byte & 0xFE) | bit;
                    fputc(byte, f_out);
                }
            }

            int byte;
            while ((byte = fgetc(f_in)) != EOF) {
                fputc(byte, f_out);
            }

            printf("\n\x1b[32m[+] SUCCESS! Message XOR encrypted and hidden inside %s\x1b[0m\n", dest_file);
            fclose(f_in);
            fclose(f_out);
        }
    } 
    else if (choice == 2) {
        char src_file[100];
        int pin;

        printf("\nEnter the BMP filename to extract from (e.g., secret_test.bmp): ");
        fgets(src_file, sizeof(src_file), stdin);
        src_file[strcspn(src_file, "\n")] = 0;

        printf("Enter the 4-digit Stego-PIN to decrypt: ");
        if (scanf("%d", &pin) != 1) pin = 1234;
        while ((c = getchar()) != '\n' && c != EOF);

        FILE *f_in = fopen(src_file, "rb");
        if (!f_in) {
            printf("\n\x1b[31m[-] Error: Could not open image file.\x1b[0m\n");
        } else {
            unsigned char header_info[54];
            fread(header_info, 1, 54, f_in);
            int pixel_offset = header_info[10] | (header_info[11] << 8) | (header_info[12] << 16) | (header_info[13] << 24);
            if (pixel_offset < 54 || pixel_offset > 2000) pixel_offset = 54;
            
            fseek(f_in, pixel_offset, SEEK_SET);

            int msg_len = 0;
            for (int i = 0; i < 32; i++) {
                int byte = fgetc(f_in);
                if (byte == EOF) break;
                int bit = byte & 1;
                msg_len |= (bit << i);
            }

            if (msg_len > 0 && msg_len < 10000) {
                char *extracted_msg = malloc(msg_len + 1);
                int xor_key = (pin % 255) + 1;

                for (int i = 0; i < msg_len; i++) {
                    char extracted_char = 0;
                    for(int j = 0; j < 8; j++) {
                        int byte = fgetc(f_in);
                        if (byte == EOF) break;
                        int bit = byte & 1;
                        extracted_char |= (bit << j);
                    }
                    extracted_msg[i] = extracted_char ^ xor_key; 
                }
                extracted_msg[msg_len] = '\0';

                printf("\n\x1b[36m[+] EXTRACED & DECRYPTED MESSAGE:\x1b[0m %s\n", extracted_msg);
                free(extracted_msg);
            } else {
                printf("\n\x1b[31m[-] Extraction failed. Wrong PIN, corrupt file, or no message exists.\x1b[0m\n");
            }
            fclose(f_in);
        }
    }

    printf("\nPress any key to return...");
    menu_getch();
}

// --- HELPER: Dynamic Window Title ---
void update_window_title() {
    char title[100];
    if (is_host) {
        sprintf(title, "CipherCore Host - Online: %d", client_count + 1);
    } else {
        sprintf(title, "CipherCore Peer - Connected");
    }
    SetConsoleTitleA(title);
}

// --- HELPER: FLOATING UI WIDGETS ---
void update_online_top() {
    EnterCriticalSection(&console_mutex);
    printf("\x1b[s"); 
    printf("\x1b[1;75H"); 
    if (is_host) {
        printf("\x1b[1;32m[ Live Online: %d ]\x1b[0m\x1b[K", client_count + 1);
    } else {
        printf("\x1b[1;32m[ Status: Connected ]\x1b[0m\x1b[K");
    }
    printf("\x1b[u"); 
    fflush(stdout);
    LeaveCriticalSection(&console_mutex);
}

void update_typing_top() {
    EnterCriticalSection(&console_mutex);
    printf("\x1b[s"); 
    printf("\x1b[2;75H"); 
    if (time(NULL) < typing_expiry) {
        printf("\x1b[90m[ %s is typing... ]\x1b[0m\x1b[K", typing_user);
    } else {
        printf("\x1b[K"); 
    }
    printf("\x1b[u"); 
    fflush(stdout);
    LeaveCriticalSection(&console_mutex);
}

void redraw_prompt() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int my_color = get_color_code(my_alias);
    printf("\x1b[%dm[%02d:%02d] [%s]:\x1b[0m ", my_color, t->tm_hour, t->tm_min, my_alias);
}

void add_to_history(const char* hex_msg) {
    EnterCriticalSection(&array_mutex);
    if (history_count < 15) {
        strcpy(chat_history[history_count], hex_msg);
        history_count++;
    } else {
        for (int i = 0; i < 14; i++) {
            strcpy(chat_history[i], chat_history[i + 1]);
        }
        strcpy(chat_history[14], hex_msg);
    }
    LeaveCriticalSection(&array_mutex);
}

// --- ASYNC FILE TRANSFER THREAD (HEX ACCUMULATOR COMPATIBLE) ---
DWORD WINAPI FileTransferThread(LPVOID lpParam) {
    char filename[256];
    strncpy(filename, (char*)lpParam, 255);
    filename[255] = '\0';
    free(lpParam); 

    FILE *fp = fopen(filename, "rb"); 
    if (!fp) {
        EnterCriticalSection(&console_mutex);
        printf("\r\x1b[2K\x1b[31m[-] File not found: %s\x1b[0m\n", filename);
        redraw_prompt();
        printf("%s", current_input);
        fflush(stdout);
        LeaveCriticalSection(&console_mutex);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long total_bytes = ftell(fp);
    rewind(fp);
    long sent_bytes = 0;

    char start_packet[150];
    char enc_start[300];
    sprintf(start_packet, "CIPHERCORE|/start %s<END>", filename);
    string_to_hex(start_packet, enc_start);
    for(int i=0; enc_start[i]!='\0'; i++) enc_start[i] = encrypt_char(enc_start[i], chat_shift_key);
    
    if (is_host) {
        EnterCriticalSection(&array_mutex);
        for (int c = 0; c < client_count; c++) send(client_sockets[c], enc_start, strlen(enc_start), 0);
        LeaveCriticalSection(&array_mutex);
    } else {
        send(current_socket, enc_start, strlen(enc_start), 0);
    }
    Sleep(100); 

    EnterCriticalSection(&console_mutex);
    printf("\r\x1b[2K\x1b[36m[*] Background uploading '%s'. Type /cancel to abort.\x1b[0m\n", filename);
    redraw_prompt();
    printf("%s", current_input);
    fflush(stdout);
    LeaveCriticalSection(&console_mutex);

    cancel_transfer = 0;
    unsigned char fbuf[200]; 
    int b_read;
    
    while ((b_read = fread(fbuf, 1, sizeof(fbuf), fp)) > 0 && chat_active && !cancel_transfer) {
        sent_bytes += b_read;
        int percent = (total_bytes > 0) ? (int)((sent_bytes * 100) / total_bytes) : 100;

        EnterCriticalSection(&console_mutex);
        printf("\x1b[s\x1b[3;75H\x1b[36m[ Uploading: %3d%% ]\x1b[0m\x1b[K\x1b[u", percent);
        fflush(stdout);
        LeaveCriticalSection(&console_mutex);

        char hex_chunk[600] = {0};
        for(int i = 0; i < b_read; i++) {
            sprintf(hex_chunk + (i * 2), "%02X", fbuf[i]); 
        }

        char file_packet[1000];
        sprintf(file_packet, "CIPHERCORE|/file %s|%s<END>", filename, hex_chunk); 
        
        char enc_packet[2500];
        string_to_hex(file_packet, enc_packet);
        for(int i = 0; enc_packet[i] != '\0'; i++) enc_packet[i] = encrypt_char(enc_packet[i], chat_shift_key);

        if (is_host) {
            EnterCriticalSection(&array_mutex);
            for (int c = 0; c < client_count; c++) send(client_sockets[c], enc_packet, strlen(enc_packet), 0);
            LeaveCriticalSection(&array_mutex);
        } else {
            send(current_socket, enc_packet, strlen(enc_packet), 0);
        }
        
        Sleep(20); 
    }
    fclose(fp);

    EnterCriticalSection(&console_mutex);
    printf("\x1b[s\x1b[3;75H\x1b[K\x1b[u"); 
    fflush(stdout);
    LeaveCriticalSection(&console_mutex);

    if (!chat_active) return 0;

    if (cancel_transfer) {
        char abort_packet[150];
        char enc_abort[300];
        sprintf(abort_packet, "CIPHERCORE|/abort %s<END>", filename);
        string_to_hex(abort_packet, enc_abort);
        for(int i = 0; enc_abort[i] != '\0'; i++) enc_abort[i] = encrypt_char(enc_abort[i], chat_shift_key);
        
        if (is_host) {
            EnterCriticalSection(&array_mutex);
            for (int c = 0; c < client_count; c++) send(client_sockets[c], enc_abort, strlen(enc_abort), 0);
            LeaveCriticalSection(&array_mutex);
        } else {
            send(current_socket, enc_abort, strlen(enc_abort), 0);
        }
        
        EnterCriticalSection(&console_mutex);
        printf("\r\x1b[2K\x1b[31m[-] Transfer forcefully aborted: %s\x1b[0m\n", filename);
        redraw_prompt();
        printf("%s", current_input);
        fflush(stdout);
        LeaveCriticalSection(&console_mutex);
        
        cancel_transfer = 0; 
        return 0;
    }

    Sleep(500); 

    char eof_packet[150];
    char enc_eof[300];
    sprintf(eof_packet, "CIPHERCORE|/eof %s<END>", filename);
    string_to_hex(eof_packet, enc_eof);
    for(int i = 0; enc_eof[i] != '\0'; i++) enc_eof[i] = encrypt_char(enc_eof[i], chat_shift_key);
    
    if (is_host) {
        EnterCriticalSection(&array_mutex);
        for (int c = 0; c < client_count; c++) send(client_sockets[c], enc_eof, strlen(enc_eof), 0);
        LeaveCriticalSection(&array_mutex);
    } else {
        send(current_socket, enc_eof, strlen(enc_eof), 0);
    }
    
    EnterCriticalSection(&console_mutex);
    printf("\r\x1b[2K\x1b[32m[+] Background transfer complete: %s\x1b[0m\n", filename);
    redraw_prompt();
    printf("%s", current_input);
    fflush(stdout);
    LeaveCriticalSection(&console_mutex);

    return 0;
}

// --- THREAD 1: Host receiving from a specific Peer ---
DWORD WINAPI HostClientHandler(LPVOID lpParam) {
    SOCKET client_sock = (SOCKET)lpParam;
    char buffer[8192];
    char temp_buffer[8192];
    char persistent_hex[32768] = {0}; 
    int bytes_received;
    char mention_tag[60];
    sprintf(mention_tag, "@%s", my_alias);

    while (chat_active) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; 
            
            memset(temp_buffer, 0, sizeof(temp_buffer));
            strcpy(temp_buffer, buffer);
            
            for (int i = 0; temp_buffer[i] != '\0'; i++) {
                temp_buffer[i] = decrypt_char(temp_buffer[i], chat_shift_key);
            }
            
            // Fill the holding tank
            if (strlen(persistent_hex) + strlen(temp_buffer) < sizeof(persistent_hex) - 1) {
                strcat(persistent_hex, temp_buffer);
            } else {
                memset(persistent_hex, 0, sizeof(persistent_hex)); // Panic flush to prevent crash
            }

            char *end_hex = strstr(persistent_hex, "3C454E443E"); 
            while (end_hex != NULL) {
                *end_hex = '\0'; 
                
                char packet_hex[8192];
                strcpy(packet_hex, persistent_hex);
                
                char decoded_msg[4096];
                hex_to_string(packet_hex, decoded_msg);
                
                char *current_msg = decoded_msg;
                
                char isolated_enc[8192];
                sprintf(isolated_enc, "%s3C454E443E", packet_hex); 
                for(int i=0; isolated_enc[i]!='\0'; i++) isolated_enc[i] = encrypt_char(isolated_enc[i], chat_shift_key);

                if (strncmp(current_msg, "CIPHERCORE|", 11) == 0) {
                    
                    if (strncmp(current_msg + 11, "/typing ", 8) == 0) {
                        strcpy(typing_user, current_msg + 19);
                        typing_expiry = time(NULL) + 2; 
                        update_typing_top();
                        
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (client_sockets[i] != client_sock) send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/start ", 7) == 0) {
                        char *fname = current_msg + 18;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname);
                        remove(save_name); 
                        
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (client_sockets[i] != client_sock) send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/file ", 6) == 0) {
                        char *delim = strchr(current_msg + 17, '|');
                        if (delim) {
                            *delim = '\0';
                            char *fname = current_msg + 17;
                            char *hex_data = delim + 1;
                            
                            char save_name[256];
                            sprintf(save_name, "recv_%s", fname);
                            
                            FILE *out = fopen(save_name, "ab"); 
                            if (out) {
                                for(int i = 0; hex_data[i] && hex_data[i+1]; i += 2) {
                                    char hex_byte[3] = {hex_data[i], hex_data[i+1], '\0'};
                                    unsigned int val;
                                    sscanf(hex_byte, "%x", &val);
                                    fputc(val, out);
                                }
                                fclose(out);
                            }
                        }
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (client_sockets[i] != client_sock) send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/eof ", 5) == 0) {
                        char *fname = current_msg + 16;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname); 
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[32m[+] File Received: %s\x1b[0m\n", save_name);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                        
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (client_sockets[i] != client_sock) send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/abort ", 7) == 0) {
                        char *fname = current_msg + 18;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname);
                        remove(save_name); 
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[31m[-] Transfer aborted by sender: %s\x1b[0m\n", save_name);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                        
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (client_sockets[i] != client_sock) send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/burn", 5) == 0) {
                        // Host silently drops malicious zero-trust attacks
                    }
                    else if (strncmp(current_msg + 11, "/alias ", 7) == 0) {
                        char new_alias[50];
                        strcpy(new_alias, current_msg + 18);
                        
                        EnterCriticalSection(&array_mutex);
                        for(int i = 0; i < client_count; i++) {
                            if(client_sockets[i] == client_sock) {
                                strcpy(client_aliases[i], new_alias);
                                
                                EnterCriticalSection(&console_mutex);
                                printf("\r\x1b[2K"); 
                                printf("\x1b[90m[+] %s joined from [%s]\x1b[0m\n", new_alias, client_ips[i]);
                                redraw_prompt();
                                printf("%s", current_input); 
                                fflush(stdout);
                                LeaveCriticalSection(&console_mutex);
                                break;
                            }
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                    else {
                        char *name_end = strstr(current_msg + 11, "]:");
                        if (name_end) {
                            char *name_start = name_end;
                            while (name_start > current_msg && *name_start != '[') name_start--;
                            if (*name_start == '[') {
                                int len = name_end - (name_start + 1);
                                EnterCriticalSection(&array_mutex);
                                for(int i = 0; i < client_count; i++) {
                                    if(client_sockets[i] == client_sock) {
                                        strncpy(client_aliases[i], name_start + 1, (len < 49 ? len : 49));
                                        client_aliases[i][(len < 49 ? len : 49)] = '\0';
                                    }
                                }
                                LeaveCriticalSection(&array_mutex);
                            }
                        }

                        int is_private = 0;
                        char target_alias[50] = {0};
                        char *msg_body = strstr(current_msg + 11, "]:\x1b[0m "); 
                        
                        if (msg_body) {
                            msg_body += 7; 
                            if (msg_body[0] == '@') {
                                is_private = 1;
                                sscanf(msg_body + 1, "%49[^ \n\r]", target_alias); 
                            }
                        }

                        if (is_private) {
                            if (strcmp(target_alias, my_alias) == 0) {
                                printf("\a"); 
                                EnterCriticalSection(&console_mutex);
                                printf("\r\x1b[2K"); 
                                printf("\x1b[35m[Private] %s\n", current_msg + 11); 
                                redraw_prompt(); 
                                printf("%s", current_input); 
                                fflush(stdout);
                                LeaveCriticalSection(&console_mutex);
                            } else {
                                EnterCriticalSection(&array_mutex);
                                for (int i = 0; i < client_count; i++) {
                                    if (strcmp(client_aliases[i], target_alias) == 0) {
                                        send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                                        break;
                                    }
                                }
                                LeaveCriticalSection(&array_mutex);
                            }
                        } else {
                            typing_expiry = 0;
                            update_typing_top();

                            if (strstr(current_msg, mention_tag) != NULL) printf("\a"); 

                            EnterCriticalSection(&console_mutex);
                            printf("\r\x1b[2K"); 
                            printf("%s\n", current_msg + 11); 
                            redraw_prompt(); 
                            printf("%s", current_input); 
                            fflush(stdout);
                            LeaveCriticalSection(&console_mutex);

                            add_to_history(isolated_enc); 

                            EnterCriticalSection(&array_mutex);
                            for (int i = 0; i < client_count; i++) {
                                if (client_sockets[i] != client_sock) {
                                    send(client_sockets[i], isolated_enc, strlen(isolated_enc), 0);
                                }
                            }
                            LeaveCriticalSection(&array_mutex);
                        }
                    }
                }
                
                memmove(persistent_hex, end_hex + 10, strlen(end_hex + 10) + 1);
                end_hex = strstr(persistent_hex, "3C454E443E");
            }
        } else {
            if (!chat_active) break; 

            char disconnected_alias[50] = "Unknown";
            
            EnterCriticalSection(&array_mutex);
            for(int i = 0; i < client_count; i++) {
                if(client_sockets[i] == client_sock) {
                    strcpy(disconnected_alias, client_aliases[i]);
                    closesocket(client_sockets[i]);
                    
                    for(int j = i; j < client_count - 1; j++) {
                        client_sockets[j] = client_sockets[j + 1];
                        strcpy(client_aliases[j], client_aliases[j + 1]);
                        strcpy(client_ips[j], client_ips[j + 1]);
                    }
                    client_count--;
                    break;
                }
            }
            LeaveCriticalSection(&array_mutex);
            
            update_window_title(); 
            update_online_top(); 

            EnterCriticalSection(&console_mutex);
            printf("\r\x1b[2K"); 
            printf("\x1b[31m[-] %s left the chat.\x1b[0m\n", disconnected_alias);
            redraw_prompt();
            printf("%s", current_input); 
            fflush(stdout);
            LeaveCriticalSection(&console_mutex);

            char sys_msg[1500];
            char hex_msg[3000];
            memset(sys_msg, 0, sizeof(sys_msg));
            memset(hex_msg, 0, sizeof(hex_msg));
            
            sprintf(sys_msg, "CIPHERCORE|\x1b[31m[-] %s left the chat.\x1b[0m<END>", disconnected_alias);
            string_to_hex(sys_msg, hex_msg);
            
            for (int k = 0; hex_msg[k] != '\0'; k++) hex_msg[k] = encrypt_char(hex_msg[k], chat_shift_key);
            
            EnterCriticalSection(&array_mutex);
            for (int k = 0; k < client_count; k++) {
                send(client_sockets[k], hex_msg, strlen(hex_msg), 0);
            }
            LeaveCriticalSection(&array_mutex);

            break; 
        }
    }
    return 0;
}

// --- THREAD 2: Host waiting for new people to join ---
DWORD WINAPI HostAcceptor(LPVOID lpParam) {
    struct sockaddr_in client;
    int c_len = sizeof(struct sockaddr_in);

    while (chat_active && client_count < MAX_CLIENTS) {
        SOCKET new_socket = accept(host_listen_socket, (struct sockaddr *)&client, &c_len);
        if (new_socket != INVALID_SOCKET) {
            
            char auth_buf[3000] = {0};
            int b = recv(new_socket, auth_buf, sizeof(auth_buf) - 1, 0);
            
            int auth_success = 0;
            char new_alias[50] = "Unknown";
            int bypassed_lobby = 0;

            if (b > 0) {
                auth_buf[b] = '\0';
                
                if (strncmp(auth_buf, "DH_INIT|", 8) == 0) {
                    long long int B;
                    char req_alias[50] = "Unknown";
                    sscanf(auth_buf + 8, "%lld|%49[^\n]", &B, req_alias);
                    
                    pending_approval = 1;
                    approval_decision = 0;
                    
                    EnterCriticalSection(&console_mutex);
                    printf("\r\x1b[2K\x1b[93m[LOBBY] User '%s' is requesting entry (DH Auto-Key). Type '/accept' or '/deny'\x1b[0m\n", req_alias);
                    redraw_prompt();
                    printf("%s", current_input);
                    fflush(stdout);
                    LeaveCriticalSection(&console_mutex);
                    
                    while(approval_decision == 0 && chat_active) {
                        Sleep(100);
                    }
                    
                    pending_approval = 0;
                    
                    if (approval_decision == -1) {
                        send(new_socket, "DH_DENY", 7, 0);
                        closesocket(new_socket);
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[31m[!] Denied entry to '%s'.\x1b[0m\n", req_alias);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                        continue; 
                    }

                    bypassed_lobby = 1; 

                    long long int P = 23, G = 5;
                    long long int a = (rand() % 10) + 2;
                    long long int A = power_mod(G, a, P);
                    long long int S = power_mod(B, a, P);

                    int cipher_key = chat_shift_key ^ (int)S;
                    char dh_reply[100];
                    sprintf(dh_reply, "DH_REPLY|%lld|%d", A, cipher_key);
                    send(new_socket, dh_reply, strlen(dh_reply), 0);

                    memset(auth_buf, 0, sizeof(auth_buf));
                    b = recv(new_socket, auth_buf, sizeof(auth_buf) - 1, 0);
                    if (b > 0) auth_buf[b] = '\0';
                }

                char temp_buffer[3000];
                strcpy(temp_buffer, auth_buf);
                
                for (int i = 0; temp_buffer[i] != '\0'; i++) {
                    temp_buffer[i] = decrypt_char(temp_buffer[i], chat_shift_key);
                }
                char decoded_auth[1500] = {0};
                hex_to_string(temp_buffer, decoded_auth);

                if (strncmp(decoded_auth, "CIPHERCORE|/alias ", 18) == 0) {
                    char *end_tag = strstr(decoded_auth, "<END>");
                    if (end_tag) *end_tag = '\0'; 
                    
                    strncpy(new_alias, decoded_auth + 18, 49);
                    new_alias[49] = '\0';

                    if (!bypassed_lobby) {
                        pending_approval = 1;
                        approval_decision = 0;
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[93m[LOBBY] User '%s' entered correct Shift Key! Type '/accept' or '/deny'\x1b[0m\n", new_alias);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                        
                        while(approval_decision == 0 && chat_active) {
                            Sleep(100);
                        }
                        
                        pending_approval = 0;
                        if (approval_decision == -1) {
                            EnterCriticalSection(&console_mutex);
                            printf("\r\x1b[2K\x1b[31m[!] Denied entry to '%s'.\x1b[0m\n", new_alias);
                            redraw_prompt();
                            printf("%s", current_input);
                            fflush(stdout);
                            LeaveCriticalSection(&console_mutex);
                            auth_success = 0; 
                        } else {
                            auth_success = 1; 
                        }
                    } else {
                        auth_success = 1; 
                    }
                }
            }

            if (!auth_success) {
                send(new_socket, "BAD", 3, 0);
                closesocket(new_socket);
                continue; 
            }

            send(new_socket, "OK", 2, 0);
            Sleep(50); 

            EnterCriticalSection(&array_mutex);
            client_sockets[client_count] = new_socket;
            strcpy(client_ips[client_count], inet_ntoa(client.sin_addr));
            strcpy(client_aliases[client_count], new_alias);
            client_count++;
            
            for(int i = 0; i < history_count; i++) {
                send(new_socket, chat_history[i], strlen(chat_history[i]), 0);
                Sleep(50); 
            }
            LeaveCriticalSection(&array_mutex);
            
            update_window_title(); 
            update_online_top(); 

            EnterCriticalSection(&console_mutex);
            printf("\r\x1b[2K"); 
            printf("\x1b[90m[+] %s joined from [%s]\x1b[0m\n", new_alias, inet_ntoa(client.sin_addr));
            redraw_prompt();
            printf("%s", current_input); 
            fflush(stdout);
            LeaveCriticalSection(&console_mutex);

            CreateThread(NULL, 0, HostClientHandler, (LPVOID)new_socket, 0, NULL);
        } else {
            if (!chat_active) break; 
        }
    }
    return 0;
}

// --- THREAD 3: Peer receiving broadcasts from Host ---
DWORD WINAPI PeerReceiver(LPVOID lpParam) {
    char buffer[8192];
    char temp_buffer[8192];
    char persistent_hex[32768] = {0}; 
    int bytes_received;
    char mention_tag[60];
    sprintf(mention_tag, "@%s", my_alias);

    while (chat_active) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(current_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; 
            
            memset(temp_buffer, 0, sizeof(temp_buffer));
            strcpy(temp_buffer, buffer);

            for (int i = 0; temp_buffer[i] != '\0'; i++) {
                temp_buffer[i] = decrypt_char(temp_buffer[i], chat_shift_key);
            }
            
            if (strlen(persistent_hex) + strlen(temp_buffer) < sizeof(persistent_hex) - 1) {
                strcat(persistent_hex, temp_buffer);
            } else {
                memset(persistent_hex, 0, sizeof(persistent_hex)); 
            }

            char *end_hex = strstr(persistent_hex, "3C454E443E"); 
            while (end_hex != NULL) {
                *end_hex = '\0'; 
                
                char packet_hex[8192];
                strcpy(packet_hex, persistent_hex);
                
                char decoded_msg[4096];
                hex_to_string(packet_hex, decoded_msg);
                
                char *current_msg = decoded_msg;

                if (strncmp(current_msg, "CIPHERCORE|", 11) == 0) {

                    if (strncmp(current_msg + 11, "/typing ", 8) == 0) {
                        strcpy(typing_user, current_msg + 19);
                        typing_expiry = time(NULL) + 2; 
                        update_typing_top();
                    }
                    else if (strncmp(current_msg + 11, "/start ", 7) == 0) {
                        char *fname = current_msg + 18;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname);
                        remove(save_name); 
                    }
                    else if (strncmp(current_msg + 11, "/file ", 6) == 0) {
                        char *delim = strchr(current_msg + 17, '|');
                        if (delim) {
                            *delim = '\0';
                            char *fname = current_msg + 17;
                            char *hex_data = delim + 1;
                            
                            char save_name[256];
                            sprintf(save_name, "recv_%s", fname);
                            
                            FILE *out = fopen(save_name, "ab"); 
                            if (out) {
                                for(int i = 0; hex_data[i] && hex_data[i+1]; i += 2) {
                                    char hex_byte[3] = {hex_data[i], hex_data[i+1], '\0'};
                                    unsigned int val;
                                    sscanf(hex_byte, "%x", &val);
                                    fputc(val, out);
                                }
                                fclose(out);
                            }
                        }
                    }
                    else if (strncmp(current_msg + 11, "/eof ", 5) == 0) {
                        char *fname = current_msg + 16;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname); 
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[32m[+] File Received: %s\x1b[0m\n", save_name);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/abort ", 7) == 0) {
                        char *fname = current_msg + 18;
                        char save_name[256];
                        sprintf(save_name, "recv_%s", fname);
                        remove(save_name); 
                        
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[31m[-] Transfer aborted by sender: %s\x1b[0m\n", save_name);
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                    }
                    else if (strncmp(current_msg + 11, "/burn", 5) == 0) {
                        EnterCriticalSection(&array_mutex);
                        history_count = 0;
                        memset(chat_history, 0, sizeof(chat_history));
                        LeaveCriticalSection(&array_mutex);

                        EnterCriticalSection(&console_mutex);
                        system("cls");
                        update_online_top();
                        printf("\n\x1b[31m[!] CHAT HISTORY BURNED BY THE HOST\x1b[0m\n\n");
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                    }
                    else {
                        typing_expiry = 0;
                        update_typing_top();

                        if (strstr(current_msg, mention_tag) != NULL) printf("\a"); 

                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K"); 
                        printf("%s\n", current_msg + 11);
                        redraw_prompt();
                        printf("%s", current_input); 
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                    }
                }

                memmove(persistent_hex, end_hex + 10, strlen(end_hex + 10) + 1);
                end_hex = strstr(persistent_hex, "3C454E443E");
            }
            
        } else {
            if (!chat_active) break; 
            
            EnterCriticalSection(&console_mutex);
            printf("\r\x1b[2K"); 
            printf("\x1b[31m[-] Disconnected from Host.\x1b[0m\n");
            LeaveCriticalSection(&console_mutex);
            chat_active = 0;
            break;
        }
    }
    return 0;
}

// --- C Audit Logger ---
void log_action(const char *username, const char *action) {
    FILE *fp = fopen(HISTORY_FILE, "a");
    if (fp != NULL) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strcspn(time_str, "\n")] = 0; 
        fprintf(fp, "[%s] %s -> %s\n", time_str, username, action);
        fclose(fp);
    }
}

// --- Simple Menu Getchar ---
char menu_getch() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    char c = 0;
    DWORD read;
    ReadConsoleA(hStdin, &c, 1, &read, NULL);
    SetConsoleMode(hStdin, mode);
    return c;
}

// --- Registration ---
void register_user() {
    char email[100], username[50], password[50];
    system("cls");
    printf("=========================================\n");
    printf("||         NEW USER REGISTRATION       ||\n");
    printf("=========================================\n\n");

    printf("Enter College Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    if (strstr(email, COLLEGE_DOMAIN) == NULL) {
        printf("\n[-] Registration Failed: Must use a valid %s email.\n", COLLEGE_DOMAIN);
        printf("Press any key to return...");
        menu_getch(); 
        return;
    }

    printf("Enter Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    for(int i = 0; username[i]; i++) {
        if(username[i] == ' ') username[i] = '_';
    }

    printf("\nDo you want the system to generate a secure password for you? (Y/N): ");
    char gen_choice = menu_getch();
    if (gen_choice == 'Y' || gen_choice == 'y') {
        const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
        srand(time(NULL));
        for (int i = 0; i < 12; i++) {
            password[i] = charset[rand() % (sizeof(charset) - 1)];
        }
        password[12] = '\0';
        printf("\n\n[+] Auto-Generated Password: %s\n", password);
        printf("[!] PLEASE SAVE THIS PASSWORD NOW. Press any key to continue registration...");
        menu_getch();
    } else {
        printf("\n\nEnter Password: ");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0;
    }

    for (int i = 0; password[i] != '\0'; i++) {
        password[i] = encrypt_char(password[i], DB_SHIFT);
    }

    FILE *fp = fopen(DB_FILE, "a");
    if (fp == NULL) {
        printf("\n[-] Database Error: Could not open file.\n");
        menu_getch(); return;
    }
    fprintf(fp, "%s %s %s\n", email, username, password);
    fclose(fp); 

    log_action(username, "Registered a new account");
    printf("\n[+] Registration Successful! You may now log in.\n");
    printf("Press any key to return...");
    menu_getch();
}

// --- Forgot Password ---
void forgot_password() {
    char target_email[100], new_password[50];
    char file_email[100], file_user[50], file_pass[50];
    int found = 0;

    system("cls");
    printf("=========================================\n");
    printf("||           PASSWORD RECOVERY         ||\n");
    printf("=========================================\n\n");
    
    printf("Enter your registered College Email: ");
    fgets(target_email, sizeof(target_email), stdin);
    target_email[strcspn(target_email, "\n")] = 0;

    FILE *fp = fopen(DB_FILE, "r");
    FILE *temp_fp = fopen("temp.txt", "w");
    if (fp == NULL || temp_fp == NULL) {
        printf("\n[-] Database Error.\n");
        if(fp) fclose(fp);
        menu_getch(); return;
    }

    printf("Enter your NEW Password: ");
    int p = 0; char ch;
    while (1) {
        ch = menu_getch(); 
        if (ch == '\r' || ch == '\n') { 
            new_password[p] = '\0'; 
            printf("\n"); break; 
        }
        else if (ch == '\b' && p > 0) { 
            p--; printf("\b \b"); 
        }
        else if (ch != '\b' && p < sizeof(new_password) - 1) { 
            new_password[p++] = ch; printf("*"); 
        }
    }

    for (int i = 0; new_password[i] != '\0'; i++) {
        new_password[i] = encrypt_char(new_password[i], DB_SHIFT);
    }

    while (fscanf(fp, "%s %s %s", file_email, file_user, file_pass) != EOF) {
        if (strcmp(target_email, file_email) == 0) {
            fprintf(temp_fp, "%s %s %s\n", file_email, file_user, new_password);
            found = 1;
        } else {
            fprintf(temp_fp, "%s %s %s\n", file_email, file_user, file_pass);
        }
    }

    fclose(fp); fclose(temp_fp);

    if (found) {
        remove(DB_FILE); rename("temp.txt", DB_FILE);
        log_action(target_email, "Reset their password");
        printf("\n[+] Password reset successfully! You can now log in.\n");
    } else {
        remove("temp.txt"); 
        printf("\n[-] Email not found in the system.\n");
    }
    printf("Press any key to return..."); 
    menu_getch();
}

// --- Authentication ---
int authenticate(char *logged_in_username) {
    char email[100], password[50];
    char file_email[100], file_user[50], file_pass[50];
    int attempts = 3;

    while (attempts > 0) {
        system("cls"); 
        printf("=========================================\n");
        printf("||       SECURE TERMINAL LOGIN         ||\n");
        printf("=========================================\n\n");
        
        printf("Email ID: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = 0; 

        FILE *fp = fopen(DB_FILE, "r");
        int email_found = 0;
        char target_pass[50]; 

        if (fp != NULL) {
            while (fscanf(fp, "%s %s %s", file_email, file_user, file_pass) != EOF) {
                if (strcmp(email, file_email) == 0) {
                    email_found = 1; 
                    strcpy(logged_in_username, file_user); 
                    strcpy(target_pass, file_pass); 
                    break;
                }
            }
            fclose(fp);
        }

        if (!email_found) {
            attempts--;
            printf("\n[-] Email not registered. Attempts remaining: %d\n", attempts);
            if (attempts > 0) {
                printf("Press any key to try again..."); menu_getch();
            } else {
                printf("Press any key to return to main menu..."); menu_getch();
            }
            continue; 
        }

        printf("Password: ");
        int p = 0; char ch;
        while (1) {
            ch = menu_getch(); 
            if (ch == '\r' || ch == '\n') { 
                password[p] = '\0'; 
                printf("\n"); break; 
            }
            else if (ch == '\b' && p > 0) { 
                p--; printf("\b \b"); 
            }
            else if (ch != '\b' && p < sizeof(password) - 1) { 
                password[p++] = ch; printf("*"); 
            }
        }

        for (int i = 0; password[i] != '\0'; i++) {
            password[i] = encrypt_char(password[i], DB_SHIFT);
        }

        if (strcmp(password, target_pass) == 0) {
            printf("\n\n[+] Authentication successful. Access granted.\n");
            printf("Press any key to enter the system...");
            menu_getch(); return 1; 
        } else {
            attempts--;
            printf("\n\n[-] Invalid password. Attempts remaining: %d\n", attempts);
            if (attempts > 0) {
                printf("\n[?] Press 'F' to Reset Password, or any key to try again...");
                char reset_choice = menu_getch();
                if (reset_choice == 'f' || reset_choice == 'F') { 
                    forgot_password(); return 0; 
                }
            } else { 
                printf("Press any key to return to main menu..."); menu_getch(); 
            }
        }
    }
    return 0; 
}

// --- Admin Panel ---
void admin_panel() {
    char password[50];
    const char *admin_pass = "HELL"; 
    
    system("cls");
    printf("=========================================\n");
    printf("||          ADMINISTRATOR LOGIN        ||\n");
    printf("=========================================\n\n");
    printf("Admin Password: ");

    int p = 0; char ch;
    while (1) {
        ch = menu_getch();
        if (ch == '\r' || ch == '\n') { 
            password[p] = '\0'; printf("\n"); break; 
        }
        else if (ch == '\b' && p > 0) { 
            p--; printf("\b \b"); 
        }
        else if (ch != '\b' && p < sizeof(password) - 1) { 
            password[p++] = ch; printf("*"); 
        }
    }

    if (strcmp(password, admin_pass) == 0) {
        system("cls");
        printf("\n[+] Admin access granted.\n\n");
        printf("=========================================\n");
        printf("||          SYSTEM AUDIT LOG           ||\n");
        printf("=========================================\n\n");

        FILE *fp = fopen(HISTORY_FILE, "r");
        if (fp == NULL) { 
            printf("[-] No history log found.\n"); 
        } else {
            char line[256];
            while (fgets(line, sizeof(line), fp) != NULL) printf("%s", line); 
            fclose(fp);
        }
    } else { 
        printf("\n[-] Intrusion detected. Access denied.\n"); 
    }
    printf("\nPress any key to return to main menu..."); 
    menu_getch();
}

// --- Secure E2EE Group Chat ---
void secure_chat(const char *current_user) {
    WSADATA wsa;
    struct sockaddr_in server;
    int choice;
    char message[1024];
    char formatted_msg[1500];
    char hex_message[3000];

    system("cls");
    printf("=========================================\n");
    printf("||       SECURE E2EE GROUP CHAT        ||\n");
    printf("=========================================\n");
    printf("||  1. Host a Secure Room (Up to 10)   ||\n");
    printf("||  2. Join a Secure Room              ||\n");
    printf("||  3. Cancel / Return to Menu         ||\n");
    printf("=========================================\n");
    printf("Enter choice: ");
    
    if (scanf("%d", &choice) != 1) { choice = 0; }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF); 

    if (choice == 3) return;

    if (choice != 1 && choice != 2) {
        printf("\n[-] Invalid choice. Press any key to return...");
        menu_getch(); return;
    }

    printf("\nEnter Shift Key (Type '0' to Auto-Generate via Diffie-Hellman): ");
    if (scanf("%d", &chat_shift_key) != 1) { chat_shift_key = 0; }
    while ((ch = getchar()) != '\n' && ch != EOF);

    printf("Enter your Chat Alias (Display Name): ");
    fgets(my_alias, sizeof(my_alias), stdin);
    my_alias[strcspn(my_alias, "\n")] = 0;
    if (strlen(my_alias) == 0) strcpy(my_alias, current_user); 

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[-] Network Error: Winsock initialization failed.\n");
        menu_getch(); return;
    }
    
    chat_active = 1;
    client_count = 0;
    history_count = 0;
    
    memset(current_input, 0, sizeof(current_input));
    input_len = 0;

    system("cls");

    if (choice == 1) { 
        is_host = 1;

        if (chat_shift_key == 0) {
            srand(time(NULL));
            chat_shift_key = (rand() % 20) + 1; 
            printf("\x1b[35m[*] Diffie-Hellman Active: Host generated a secure hidden Session Key.\x1b[0m\n");
        }

        host_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(8888); 

        bind(host_listen_socket, (struct sockaddr *)&server, sizeof(server));
        listen(host_listen_socket, MAX_CLIENTS);

        update_window_title();
        printf("\x1b[32m[+] Group Room created. Listening on Port 8888...\x1b[0m\n");
        printf("\x1b[90m[!] Host Commands: /list, /kick [Alias], /accept, /deny, /burn\x1b[0m\n");
        printf("\x1b[90m[!] Transfer File: /send [filename.ext] | Abort Transfer: /cancel\x1b[0m\n");
        printf("Type 'x*x' to leave.\n");
        printf("----------------------------------------------------------------------\n");
        update_online_top(); 
        
        CreateThread(NULL, 0, HostAcceptor, NULL, 0, NULL);
        
    } else if (choice == 2) { 
        is_host = 0;
        int use_dh = (chat_shift_key == 0); 

        char host_input[100];
        int target_port;
        struct hostent *he; 

        printf("\nEnter Host's Domain or IP: ");
        fgets(host_input, sizeof(host_input), stdin);
        host_input[strcspn(host_input, "\n")] = 0;

        printf("Enter Target Port: ");
        if (scanf("%d", &target_port) != 1) { target_port = 8888; }
        while ((ch = getchar()) != '\n' && ch != EOF);

        if ((he = gethostbyname(host_input)) == NULL) {
            printf("\x1b[31m[-] C DNS Error: Could not resolve hostname.\x1b[0m\n");
            menu_getch(); return;
        }

        current_socket = socket(AF_INET, SOCK_STREAM, 0);
        server.sin_family = AF_INET;
        server.sin_port = htons(target_port);
        server.sin_addr = *((struct in_addr *)he->h_addr);

        printf("\n[*] Connecting to %s on Port %d...\n", host_input, target_port);
        if (connect(current_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
            printf("\x1b[31m[-] Connection failed. Check Domain/Port.\x1b[0m\n");
            menu_getch(); return;
        }

        if (use_dh) {
            printf("\x1b[35m[*] Initiating Diffie-Hellman Key Exchange...\x1b[0m\n");
            long long int P = 23, G = 5;
            srand(time(NULL));
            long long int b = (rand() % 10) + 2;
            long long int B = power_mod(G, b, P);

            char dh_msg[150];
            sprintf(dh_msg, "DH_INIT|%lld|%s", B, my_alias);
            send(current_socket, dh_msg, strlen(dh_msg), 0);

            printf("\x1b[33m[*] Waiting for Host to approve your connection in Lobby...\x1b[0m\n");

            char dh_reply[150] = {0};
            recv(current_socket, dh_reply, sizeof(dh_reply) - 1, 0);

            if (strncmp(dh_reply, "DH_DENY", 7) == 0) {
                printf("\x1b[31m[-] Entry Denied: The Host rejected your request.\x1b[0m\n");
                closesocket(current_socket);
                menu_getch(); return;
            }
            else if (strncmp(dh_reply, "DH_REPLY|", 9) == 0) {
                long long int A;
                int cipher_key;
                sscanf(dh_reply + 9, "%lld|%d", &A, &cipher_key);
                long long int S = power_mod(A, b, P);
                chat_shift_key = cipher_key ^ (int)S; 
                printf("\x1b[32m[+] Auto-Key Synced: Session securely established!\x1b[0m\n");
            } else {
                printf("\x1b[31m[-] Handshake failed.\x1b[0m\n");
                closesocket(current_socket); return;
            }
        } else {
            printf("\x1b[33m[*] Sending Manual Shift Key & Waiting in Lobby...\x1b[0m\n");
        }

        char auth_msg[150];
        char hex_auth[300];
        sprintf(auth_msg, "CIPHERCORE|/alias %s<END>", my_alias);
        string_to_hex(auth_msg, hex_auth);
        
        for (int i = 0; hex_auth[i] != '\0'; i++) {
            hex_auth[i] = encrypt_char(hex_auth[i], chat_shift_key);
        }
        send(current_socket, hex_auth, strlen(hex_auth), 0);

        char verify_buf[100] = {0};
        recv(current_socket, verify_buf, sizeof(verify_buf) - 1, 0);

        if (strncmp(verify_buf, "OK", 2) != 0) {
            printf("\x1b[31m[-] Access Denied: Incorrect Room Password OR Host Rejected You!\x1b[0m\n");
            closesocket(current_socket);
            menu_getch(); return;
        }

        system("cls");
        update_window_title();
        printf("\x1b[32m[+] Connected to the Group Chat! Receiving history...\x1b[0m\n");
        printf("\x1b[90m[!] Tools: /send [file], /cancel, /burn (Erase History)\x1b[0m\n");
        printf("Type 'x*x' to leave.\n"); 
        printf("----------------------------------------------------------------------\n");
        update_online_top(); 

        CreateThread(NULL, 0, PeerReceiver, NULL, 0, NULL);
    }

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalInMode;
    GetConsoleMode(hStdin, &originalInMode);
    SetConsoleMode(hStdin, originalInMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    WCHAR high_surrogate = 0;

    EnterCriticalSection(&console_mutex);
    redraw_prompt(); 
    LeaveCriticalSection(&console_mutex);
    
    while (chat_active) {
        if (typing_expiry > 0 && time(NULL) > typing_expiry) {
            typing_expiry = 0;
            update_typing_top();
        }

        DWORD unread;
        GetNumberOfConsoleInputEvents(hStdin, &unread);
        
        if (unread > 0) {
            INPUT_RECORD irInBuf[128];
            DWORD cNumRead;
            ReadConsoleInputW(hStdin, irInBuf, 128, &cNumRead);

            int ui_needs_update = 0;
            int message_ready = 0;

            for (DWORD i = 0; i < cNumRead; i++) {
                if (irInBuf[i].EventType == KEY_EVENT && irInBuf[i].Event.KeyEvent.bKeyDown) {
                    WCHAR wc = irInBuf[i].Event.KeyEvent.uChar.UnicodeChar;

                    if (wc == 0) continue; 

                    if (wc == L'\r' || wc == L'\n') {
                        message_ready = 1;
                        break;
                    } 
                    else if (wc == L'\b' || wc == 127) {
                        if (input_len > 0) {
                            do { input_len--; } while (input_len > 0 && (current_input[input_len] & 0xC0) == 0x80);
                            current_input[input_len] = '\0';
                            ui_needs_update = 1;
                        }
                    } 
                    else if (wc >= 32) {
                        WCHAR utf16_buf[2] = {0};
                        int utf16_len = 1;

                        if (wc >= 0xD800 && wc <= 0xDBFF) {
                            high_surrogate = wc;
                            continue;
                        } else if (wc >= 0xDC00 && wc <= 0xDFFF) {
                            if (high_surrogate != 0) {
                                utf16_buf[0] = high_surrogate;
                                utf16_buf[1] = wc;
                                utf16_len = 2;
                                high_surrogate = 0;
                            } else {
                                continue;
                            }
                        } else {
                            utf16_buf[0] = wc;
                            high_surrogate = 0;
                        }

                        char utf8_char[5] = {0};
                        int bytes = WideCharToMultiByte(CP_UTF8, 0, utf16_buf, utf16_len, utf8_char, 4, NULL, NULL);
                        if (bytes > 0 && input_len + bytes < 1000) {
                            strcpy(&current_input[input_len], utf8_char);
                            input_len += bytes;
                            ui_needs_update = 1;

                            if (time(NULL) > last_sent_typing) {
                                char typ_msg[150];
                                char hex_typ[300];
                                sprintf(typ_msg, "CIPHERCORE|/typing %s<END>", my_alias);
                                string_to_hex(typ_msg, hex_typ);
                                for (int k = 0; hex_typ[k] != '\0'; k++) hex_typ[k] = encrypt_char(hex_typ[k], chat_shift_key);
                                
                                if (is_host) {
                                    EnterCriticalSection(&array_mutex);
                                    for (int k = 0; k < client_count; k++) send(client_sockets[k], hex_typ, strlen(hex_typ), 0);
                                    LeaveCriticalSection(&array_mutex);
                                } else {
                                    send(current_socket, hex_typ, strlen(hex_typ), 0);
                                }
                                last_sent_typing = time(NULL);
                            }
                        }
                    }
                }
            }

            if (message_ready) {
                EnterCriticalSection(&console_mutex);
                printf("\n");
                LeaveCriticalSection(&console_mutex);
                
                if (input_len == 0) {
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }

                strcpy(message, current_input);
                memset(current_input, 0, sizeof(current_input));
                input_len = 0;

                if (strcmp(message, "x*x") == 0) {
                    chat_active = 0; break;
                }

                if (strcmp(message, "/accept") == 0) {
                    if (is_host && pending_approval) {
                        approval_decision = 1;
                    }
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }
                if (strcmp(message, "/deny") == 0) {
                    if (is_host && pending_approval) {
                        approval_decision = -1;
                    }
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }

                if (strcmp(message, "/cancel") == 0) {
                    cancel_transfer = 1;
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue;
                }

                if (strcmp(message, "/clear") == 0) {
                    EnterCriticalSection(&console_mutex);
                    system("cls");
                    update_online_top();
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }

                if (strcmp(message, "/burn") == 0) {
                    if (!is_host) {
                        EnterCriticalSection(&console_mutex);
                        printf("\r\x1b[2K\x1b[31m[-] Access Denied: Only the Room Host can use /burn.\x1b[0m\n");
                        redraw_prompt();
                        printf("%s", current_input);
                        fflush(stdout);
                        LeaveCriticalSection(&console_mutex);
                        continue;
                    }

                    char burn_packet[150];
                    char enc_burn[300];
                    sprintf(burn_packet, "CIPHERCORE|/burn<END>");
                    string_to_hex(burn_packet, enc_burn);
                    for(int i = 0; enc_burn[i] != '\0'; i++) enc_burn[i] = encrypt_char(enc_burn[i], chat_shift_key);
                    
                    if (is_host) {
                        EnterCriticalSection(&array_mutex);
                        for (int c = 0; c < client_count; c++) send(client_sockets[c], enc_burn, strlen(enc_burn), 0);
                        LeaveCriticalSection(&array_mutex);
                    } else {
                        send(current_socket, enc_burn, strlen(enc_burn), 0);
                    }
                    
                    EnterCriticalSection(&array_mutex);
                    history_count = 0;
                    memset(chat_history, 0, sizeof(chat_history));
                    LeaveCriticalSection(&array_mutex);
                    
                    EnterCriticalSection(&console_mutex);
                    system("cls");
                    update_online_top();
                    printf("\n\x1b[31m[!] CHAT HISTORY BURNED BY YOU\x1b[0m\n\n");
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue;
                }

                if (is_host && strcmp(message, "/list") == 0) {
                    EnterCriticalSection(&console_mutex);
                    printf("\n--- CURRENT PEERS (%d) ---\n", client_count);
                    for(int i = 0; i < client_count; i++) {
                        printf("%d. %s [%s]\n", i + 1, client_aliases[i], client_ips[i]);
                    }
                    printf("--------------------------\n");
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }

                if (is_host && strncmp(message, "/kick ", 6) == 0) {
                    char *target = message + 6;
                    EnterCriticalSection(&array_mutex);
                    int found = 0;
                    for(int i = 0; i < client_count; i++) {
                        if(strcmp(client_aliases[i], target) == 0) {
                            closesocket(client_sockets[i]); 
                            
                            EnterCriticalSection(&console_mutex);
                            printf("\x1b[2K\r\x1b[33m[!] Booted %s. Connection severed.\x1b[0m\n", target);
                            LeaveCriticalSection(&console_mutex);
                            
                            for(int j = i; j < client_count - 1; j++) {
                                client_sockets[j] = client_sockets[j + 1];
                                strcpy(client_aliases[j], client_aliases[j + 1]);
                                strcpy(client_ips[j], client_ips[j + 1]);
                            }
                            client_count--; found = 1; break;
                        }
                    }
                    if(!found) {
                        EnterCriticalSection(&console_mutex);
                        printf("\x1b[2K\r\x1b[31m[-] User not found.\x1b[0m\n");
                        LeaveCriticalSection(&console_mutex);
                    }
                    LeaveCriticalSection(&array_mutex);
                    update_window_title();
                    update_online_top();
                    
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt();
                    LeaveCriticalSection(&console_mutex);
                    continue; 
                }

                if (strncmp(message, "/send ", 6) == 0) {
                    char *filename = message + 6;
                    char *pass_name = malloc(strlen(filename) + 1);
                    strcpy(pass_name, filename);
                    
                    CreateThread(NULL, 0, FileTransferThread, (LPVOID)pass_name, 0, NULL);
                    continue;
                }

                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                int my_color = get_color_code(my_alias);

                memset(formatted_msg, 0, sizeof(formatted_msg));
                sprintf(formatted_msg, "CIPHERCORE|\x1b[%dm[%02d:%02d] [%s]:\x1b[0m %s<END>", my_color, t->tm_hour, t->tm_min, my_alias, message);
                
                memset(hex_message, 0, sizeof(hex_message));
                string_to_hex(formatted_msg, hex_message);

                for (int i = 0; hex_message[i] != '\0'; i++) hex_message[i] = encrypt_char(hex_message[i], chat_shift_key);

                if (is_host) {
                    int host_is_private = 0;
                    char host_target[50] = {0};
                    if (message[0] == '@') {
                        host_is_private = 1;
                        sscanf(message + 1, "%49[^ \n\r]", host_target);
                    }

                    if (host_is_private) {
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            if (strcmp(client_aliases[i], host_target) == 0) {
                                send(client_sockets[i], hex_message, strlen(hex_message), 0);
                                break;
                            }
                        }
                        LeaveCriticalSection(&array_mutex);
                    } else {
                        add_to_history(hex_message);
                        EnterCriticalSection(&array_mutex);
                        for (int i = 0; i < client_count; i++) {
                            send(client_sockets[i], hex_message, strlen(hex_message), 0);
                        }
                        LeaveCriticalSection(&array_mutex);
                    }
                } else {
                    send(current_socket, hex_message, strlen(hex_message), 0);
                }
                
                if (chat_active) {
                    EnterCriticalSection(&console_mutex);
                    redraw_prompt(); 
                    LeaveCriticalSection(&console_mutex);
                }

            } 
            else if (ui_needs_update) {
                EnterCriticalSection(&console_mutex);
                printf("\r\x1b[2K"); 
                redraw_prompt();
                printf("%s", current_input);
                fflush(stdout); 
                LeaveCriticalSection(&console_mutex);
            }
        }
        Sleep(10); 
    }

    SetConsoleMode(hStdin, originalInMode); 

    if (is_host) {
        closesocket(host_listen_socket); 
        for(int i = 0; i < client_count; i++) closesocket(client_sockets[i]);
    } else {
        closesocket(current_socket);
    }
    
    Sleep(100); 

    SetConsoleTitleA("CipherCore Terminal");
    WSACleanup();
    printf("\n[!] Chat terminated. Press any key to return to main menu...\n");
    menu_getch();
}

// --- Main Program State Machine ---
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    SetConsoleTitleA("CipherCore Terminal");

    InitializeCriticalSection(&console_mutex);
    InitializeCriticalSection(&array_mutex);

    int start_choice;
    char current_user[100]; 

    while (1) {
        system("cls");
        printf("=========================================\n");
        printf("||           DENCRYPTION CHAT          ||\n");
        printf("=========================================\n");
        printf("||  1. Login                           ||\n");
        printf("||  2. Register New User               ||\n");
        printf("||  3. Admin Panel                     ||\n"); 
        printf("||  4. Exit to Windows                 ||\n"); 
        printf("=========================================\n");
        printf("Enter choice: ");
        
        if (scanf("%d", &start_choice) != 1) start_choice = 0; 
        int c; while ((c = getchar()) != '\n' && c != EOF); 

        if (start_choice == 4) {
            DeleteCriticalSection(&console_mutex); 
            DeleteCriticalSection(&array_mutex);
            return 0; 
        } else if (start_choice == 3) admin_panel(); 
        else if (start_choice == 2) register_user();
        else if (start_choice == 1) {
            if (authenticate(current_user)) {
                log_action(current_user, "Logged in to the system");

                while (1) {
                    system("cls"); 
                    printf("=========================================\n");
                    printf("||             MAIN MENU               ||\n");
                    printf("=========================================\n");
                    printf("|| Active User: %-22s ||\n", current_user); 
                    printf("=========================================\n");
                    printf("||  1. Encrypt a message               ||\n");
                    printf("||  2. Decrypt a message               ||\n");
                    printf("||  3. Secure Live Chat (E2EE)         ||\n");
                    printf("||  4. Logout                          ||\n");
                    printf("||  5. Encrypted LSB Steganography     ||\n");
                    printf("=========================================\n");
                    printf("Enter choice: ");
                    
                    int choice; 
                    if (scanf("%d", &choice) != 1) choice = 0; 
                    int ch; while ((ch = getchar()) != '\n' && ch != EOF); 

                    if (choice == 4) {
                        log_action(current_user, "Logged out");
                        printf("\nLogging out...\n"); 
                        break; 
                    }
                    if (choice < 1 || choice > 5) {
                        printf("\n[-] Invalid choice! Press any key to try again...");
                        menu_getch(); continue; 
                    }
                    
                    if (choice == 5) {
                        steganography_demo(); continue;
                    }

                    if (choice == 3) {
                        log_action(current_user, "Used Secure Live Chat");
                        secure_chat(current_user); continue;
                    }

                    system("cls");
                    if (choice == 1) {
                        printf("=========================================\n||          ENCRYPTION MODE            ||\n=========================================\n\n");
                        log_action(current_user, "Used Encryption tool");
                    } else {
                        printf("=========================================\n||          DECRYPTION MODE            ||\n=========================================\n\n");
                        log_action(current_user, "Used Decryption tool");
                    }

                    int shift;
                    printf("Enter numeric shift key: "); 
                    if (scanf("%d", &shift) != 1) shift = 0; 
                    int ch_buf; while ((ch_buf = getchar()) != '\n' && ch_buf != EOF);

                    printf("\n[!] Type 'EXIT' at any time to return to the main menu.\n");

                    while (1) {
                        char text[1024];
                        char hex_buf[2048];
                        char final_buf[1024];

                        printf("\nEnter text: ");
                        fgets(text, sizeof(text), stdin);
                        text[strcspn(text, "\n")] = 0;

                        if (strcmp(text, "EXIT") == 0 || strcmp(text, "exit") == 0) {
                            break;
                        }
                        
                        if (strlen(text) == 0) continue;

                        if (choice == 1) {
                            string_to_hex(text, hex_buf);
                            for (int i = 0; hex_buf[i] != '\0'; i++) hex_buf[i] = encrypt_char(hex_buf[i], shift);
                            printf("[+] Encrypted Result: %s\n", hex_buf);
                        } else {
                            for (int i = 0; text[i] != '\0'; i++) text[i] = decrypt_char(text[i], shift);
                            hex_to_string(text, final_buf);
                            printf("[+] Decrypted Result: %s\n", final_buf);
                        }
                    }
                } 
            } 
        } else {
            printf("\n[-] Invalid choice! Press any key to try again...");
            menu_getch();
        }
    } 
    return 0; 
}