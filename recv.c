#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {
  msg t;
  my_pkt p;
  long filesize, read_so_far, fd;
  char filename[MSGSIZE];

  init(HOST, PORT);
  /* Asteptam pentru numele fisierului */
  memset(t.payload, 0, sizeof(t.payload));
  if (recv_message(&t) < 0) {
    perror("[RECEIVER] Receive message");
    return -1;
  }
  p = *((my_pkt *)t.payload);
  /* Verificam daca s-a trimis numele fisierului */
  if (p.type != TYPE1) {
    perror("[RECEIVER] Receive message");
    return -1;
  }
  /* Extragem numele fisierului pe care l-am primit */
  memcpy(filename, p.payload, t.len - sizeof(int));
  /* Numele fisierului ce va fi creat pentru a scrie continutul */
  char filename_acc[MSGSIZE];
  sprintf(filename_acc, "recv_%s", filename);
  memset(t.payload, 0, sizeof(t.payload));
  memset(p.payload, 0, sizeof(p.payload));
  /* Trimitem ACK pentru primirea numelui fisierului */
  p.type = TYPE4;
  memcpy(p.payload, ACK_T1, strlen(ACK_T1));
  t.len = strlen(p.payload) + 1 + sizeof(int);
  memcpy(t.payload, &p, t.len);
  send_message(&t);
  /* Asteptam pentru dimensiunea fisierului */
  memset(t.payload, 0, sizeof(t.payload));
  if (recv_message(&t) < 0) {
    perror("Receive message");
    return -1;
  }
  p = *((my_pkt *)t.payload);
  if (p.type != TYPE2) {
    perror("[RECEIVER] Receive message");
    return -1;
  }
  memcpy(&filesize, p.payload, sizeof(int));
  memset(t.payload, 0, sizeof(t.payload));
  memset(p.payload, 0, sizeof(p.payload));
  /* Trimitem ACK pentru dimensiunea fisierului */
  p.type = TYPE4;
  memcpy(p.payload, ACK_T2, strlen(ACK_T2));
  t.len = strlen(p.payload) + sizeof(int);
  memcpy(t.payload, &p, t.len);
  send_message(&t);

  read_so_far = 0;
  // Cream fisierul in care scriem mesajul trimis din send.c
  fd = open(filename_acc, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  /* Asteptam pentru continutul fisierului - TYPE3 messages */
  while (read_so_far < filesize) {
    memset(t.payload, 0, sizeof(t.payload));
    memset(p.payload, 0, sizeof(p.payload));
    if (recv_message(&t) < 0) {
      perror("[RECEIVER] Receive messagen");
      return -1;
    }
    p = *((my_pkt *)t.payload);
    if (p.type != TYPE3) {
      perror("[RECEIVER] Receive message");
      return -1;
    }
    read_so_far += (t.len - sizeof(int));
    /* Scriem mesajele primite in fisier */
    write(fd, p.payload, t.len - sizeof(int));
    memset(t.payload, 0, sizeof(t.payload));
    memset(p.payload, 0, sizeof(p.payload));

    if (read_so_far != filesize) {
      p.type = TYPE4;
    } else {
      p.type = FINISH;
    }
    memcpy(p.payload, ACK_T3, strlen(ACK_T3));
    t.len = strlen(p.payload) + sizeof(int);
    memcpy(t.payload, &p, t.len);
    /* Trimtem ACK-uri pentru confirmarea mesajelor primite */
    send_message(&t);
  }
  printf("[RECEIVER] All done.\n");
  close(fd);
  return 0;
}
