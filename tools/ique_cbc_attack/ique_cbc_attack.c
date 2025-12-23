#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define AES_BLOCK_SIZE 0x10

char char_lut[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int get_hex_from_char(char in)
{
  in = toupper(in);
  int index = 0;
  while(index < 16)
  {
    char c = char_lut[index];
    if(c == in)
      break;
    index++;
  }
  return index;
}

uint8_t *get_hex_array(char *in)
{
  size_t len = strlen(in);

  uint8_t *out_buf = (uint8_t *)malloc(len / 2);
  memset(out_buf, 0, len / 2);

  for(int i = 0; i < len / 2; i++)
  {
    out_buf[i] |= get_hex_from_char(in[i * 2]) << 4;
    out_buf[i] |= get_hex_from_char(in[i * 2 + 1]) & 0xF;
  }
  return out_buf;
}

void print_help(char *name)
{
  printf("Usage:\n");
  printf("%s args\n", name);
  printf("  -p  - plaintext data passed in from command line (in hex, no 0x at the start)\n");
  printf("  -pf - plaintext data from file.\n");
  printf("  -r  - \"recrypted\" file, the encrypted game file to be injected into.\n");
  printf("  -d  - data to inject, passed in from command line (in hex, no 0x at the start)\n");
  printf("  -df - data to inject from file. Data to inject must be at the same offset in this file as the offset you'd like to inject it to.\n");
  printf("  -o  - offset to inject data to (in hex, no 0x at the start)\n");
  printf("example: %s -p 00000000000000000000000000000000 -d deadbeefdeadbeefdeadbeefdeadbeef -r 005d1870.rec -o 1000\n", name);
  printf("this injects deadbeefdeadbeefdeadbeefdeadbeef over plaintext 00000000000000000000000000000000 at offset 0x1000 in encrypted file 005d1870.rec\n");
  exit(1);
}

void parse_args(int argc, char **argv, uint8_t **pt_data, size_t *pt_size, char **pt_path, char **rec_path, char **user_file, uint8_t **user_data, size_t *size, size_t *offset)
{
  for(int i = 0; i < argc; i++)
  {
    if(!strcmp("-p", argv[i]) && i + 1 < argc)
    {
      *pt_data = get_hex_array(argv[i + 1]);
      *pt_size = strlen(argv[i + 1]) / 2;
    }
    if(!strcmp("-pf", argv[i]) && i + 1 < argc)
      *pt_path = argv[i + 1];
    if(!strcmp("-r", argv[i]) && i + 1 < argc)
      *rec_path = argv[i + 1];
    if(!strcmp("-d", argv[i]) && i + 1 < argc)
    {
      *user_data = get_hex_array(argv[i + 1]);
      *size = strlen(argv[i + 1]) / 2;
    }
    if(!strcmp("-df", argv[i]) && i + 1 < argc)
      *user_file = argv[i + 1];
    if(!strcmp("-o", argv[i]) && i + 1 < argc)
      *offset = strtol(argv[i + 1], NULL, 16);
    if(argc < 9)
      print_help(argv[0]);
  }
}

void inject_block(uint8_t *original_data, uint8_t *recrypted, uint8_t *user_data, uint8_t *out, size_t offset)
{
  for(int i = 0; i < AES_BLOCK_SIZE; i++)
    out[i] = user_data[i] ^ (original_data[i] ^ recrypted[i]);
}

int main(int argc, char **argv)
{
  char *plaintext = 0;
  char *recrypted = 0;
  char *user_file = 0;
  uint8_t *pt_data = 0;
  uint8_t *user_data = 0;
  size_t pt_size = 0;
  size_t size = 0;
  size_t offset = 0;

  printf("AES-CBC attack, by stuckpixel\n");

  parse_args(argc, argv, &pt_data, &pt_size, &plaintext, &recrypted, &user_file, &user_data, &size, &offset);

  if(pt_size != size && pt_data && user_data)
  {
    printf("plaintext size and data size must be the same!\n");
    exit(1);
  }

  if(user_file && user_data)
  {
    printf("please specify either a data file, or just data, not both!\n");
    exit(1);
  }

  if(user_file)
  {
    struct stat file_stat;
    if(stat(user_file, &file_stat) < 0)
    {
      printf("Failed to get payload size!\n");
      exit(1);
    }

    size = file_stat.st_size;

    FILE *user_f = fopen(user_file, "rb");
    user_data = (uint8_t *)malloc(size);
    if(!user_f || !user_data)
    {
      printf("Failed to open file or allocate enough space!\n");
      exit(1);
    }
    fread(user_data, 1, size, user_f);
    fclose(user_f);
  }

  if(offset % AES_BLOCK_SIZE)
  {
    printf("Offset not AES_BLOCK_SIZE byte aligned!\n");
    exit(1);
  }

  if(offset == 0)
  {
    printf("Offset must be at least 0x10!");
    exit(1);
  }
  
  FILE *plaintext_f = NULL;
  if(!pt_data)
  {
    plaintext_f = fopen(plaintext, "rb"); //plaintext is a path passed in by the user
    if(!plaintext_f)
    {
      printf("Failed to open the plaintext file!\n");
      exit(1);
    }
  }

  FILE *recrypted_f = fopen(recrypted, "rb+"); //recrypted is a path to the .rec file, passed by user. Open the file for both reading and writing

  if(!recrypted_f)
  {
    printf("Failed to open the encrypted file!\n");
    exit(1);
  }

  uint8_t original_data[AES_BLOCK_SIZE];
  uint8_t prev_block_data[AES_BLOCK_SIZE];
  uint8_t modified_rc_data[AES_BLOCK_SIZE];

  for(int i = 0; i < size / AES_BLOCK_SIZE; i++)
  {
    memset(original_data, 0, AES_BLOCK_SIZE);
    memset(prev_block_data, 0, AES_BLOCK_SIZE);
    memset(modified_rc_data, 0, AES_BLOCK_SIZE);

    if(!pt_data)
    {
      fseek(plaintext_f, offset, SEEK_SET); //seek to the offset to modify
      fread(original_data, 1, AES_BLOCK_SIZE, plaintext_f); //read the data we're after
    }
    else
    {
      memcpy(original_data, &pt_data[i * 0x20], AES_BLOCK_SIZE); // skip every other block, since we can only control every other block
    }

    fseek(recrypted_f, offset - AES_BLOCK_SIZE, SEEK_SET); //seek to block - 1

    fread(prev_block_data, 1, AES_BLOCK_SIZE, recrypted_f); //get the original encrypted data
    fseek(recrypted_f, offset - AES_BLOCK_SIZE, SEEK_SET); //seek back since the position was modified by reading

    inject_block(original_data, prev_block_data, user_data, modified_rc_data, offset);

    fwrite(modified_rc_data, 1, AES_BLOCK_SIZE, recrypted_f); //write the data to the .rec file
    user_data += AES_BLOCK_SIZE;

    offset += AES_BLOCK_SIZE * 2;
  }

  /* free buffers */
  if(pt_data)
    free(pt_data);
  if(user_data)
    free(user_data - size);

  /* close files */

  if(plaintext_f != NULL)
    fclose(plaintext_f);
  fclose(recrypted_f);

  printf("successfully injected %d blocks!\n", size / AES_BLOCK_SIZE);

  return 0;
}
