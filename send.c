#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"
#include "link_emulator/lib.h"
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000
#define FRAME_SIZE 1404
#define BITS_NO 8

int main(int argc, char** argv) {
  msg t;
  my_pkt p;
  long fd, count, filesize;
  struct stat f_status;
  char buffer[MSGSIZE];
  long bandwidth, delay, window;

  init(HOST, PORT);
  printf("[SENDER] File to send: %s\n", argv[1]);
  /* Retinem descriptorul fisierului deschis 
  reprezentat de parametrul transmis */
  fd = open(argv[1], O_RDONLY);
  fstat(fd, &f_status);
  /* Viteza de transmisie (V) - bandwidth */
  bandwidth = atol(argv[2]);
  /* Timpul de propagare (TP) - delay */
  delay = atol(argv[3]);
  /* Cantitatea de informatie aflata in zbor la un anumit moment de timp */
  long bdp = bandwidth * delay * 1000;
  /* window reprezinta numarul maxim de 
  cadre neconfirmate la orice moment de timp*/
  window = bdp / (FRAME_SIZE * BITS_NO);
  /* Retinem dimensiunea fisierului care urmeaza sa-l transmitem */
  filesize = (long)f_status.st_size;
  memset(t.payload, 0, sizeof(t.payload));
  memset(p.payload, 0, sizeof(p.payload));
  // TYPE1 reprezinta trimiterea pentru numele de fisier (are valoarea 1)
  p.type = TYPE1;
  memcpy(p.payload, argv[1], strlen(argv[1]));
  t.len = sizeof(int) + strlen(argv[1]);
  memcpy(t.payload, &p, t.len);
  /* Trimitem numele fisierului de transmis */
  send_message(&t);
  /* ASteptam ACK-ul pentru numele fisierului */
  if (recv_message(&t) < 0) {
    perror("[SENDER] Receive error");
    return -1;
  }
  /* Verificam confirmarea din recv.c (p.type este TYPE4) */
  p = *((my_pkt *) t.payload);
  if (p.type != TYPE4) {
    perror("[SENDER] Receive error");
    return -1;
  }
  memset(t.payload, 0, sizeof(t.payload));
  memset(p.payload, 0, sizeof(p.payload));
  /* Vom trimite dimensiunea fisierului - TYPE2 message */
  p.type = TYPE2;
  /* Adaugam lungimea fisierului */
  memcpy(p.payload, &filesize, sizeof(int));
  t.len = sizeof(int) * 2;
  /* Se copiaza bit cu bit structura p in t.payload */
  memcpy(t.payload, &p, t.len);
  send_message(&t);
  /* Asteptam ACK pentru primirea lungimii fisierului */
  if (recv_message(&t) < 0) {
    perror("[SENDER] Receive error");
    return -1;
  }
  /* Verificam ACK TYPE4 pentru confirmarea primirii mesajului */
  p = *((my_pkt *)t.payload);
  if (p.type != TYPE4) {
    perror("[SENDER] Receive error");
    return -1;
  }
  /* Trimitem continutul fisierului - TYPE3 MESSAGES */
  long frames = 0;
  msg messages[COUNT];
  /* Cat timp citim din fisier, salvam mesajele de trimis intr-un buffer
   pentru a le trimite cu ajutorul ferestrei glisante */
  while ((count = read(fd, buffer, MSGSIZE - sizeof(int))) > 0) {
    memset(messages[frames].payload, 0, sizeof(messages[frames].payload));
    memset(p.payload, 0, sizeof(p.payload));
    /* Trimitem continutul fisierului - TYPE3 MESSAGES */
    p.type = TYPE3;
    memcpy(p.payload, buffer, count);
    messages[frames].len = sizeof(int) + count;
    memcpy(messages[frames].payload, &p, messages[frames].len);
    frames++;
  }
 long res = 0, i;
 /* Facem min dintre window si numarul de pachete,daca exista mai puține pachete
  decât dimensiunea window, încearcă sa trimita pachete care nu exista */
  if (frames < window) {
    window = frames;
  }
  /* Punem pe fir un numar de cadre egal cu dimensiunea ferestrei */
  for (i = 0; i < window; i++) {
    res = send_message(&messages[i]);
    if (res < 0) {
      perror("[SENDER] Send error. Exiting.\n");
      return -1;
    }
  }
  /* Asteptam confirmarile deoarece pentru fiecare confirmare putem 
  sa introducem un nou cadru in retea. (putem trimite urmatorul cadrul) */
  long count_window = window;
  for (i = 0; i < frames - window; i++) {
    /* Asteptam ACK pentru primirea mesajului din buffer */
    res = recv_message(&t);
    if (res < 0) {
      perror("[SENDER] Receive error. Exiting.\n");
      return -1;
    }
    p = *((my_pkt *)t.payload);
    /* Verificam ACK TYPE4 pentru confirmarea primirii mesajului */
    if (p.type != TYPE4) {
      perror("[SENDER] Receive error");
      return -1;
    }
    /* Introducem un nou cadru in retea, putem trimite urmatorul cadru */
    res = send_message(&messages[count_window]);
    if (res < 0) {
      perror("[SENDER] Send error. Exiting.\n");
      return -1;
    }
    count_window++;
  }
  /* Asteptam ACK-urile pentru celelalte window mesaje care au ramas pe fir */
  for (i = 0; i < window; i++) {
    memset(t.payload, 0, sizeof(t.payload));
    res = recv_message(&t);
    if (res < 0) {
      perror("[SENDER] Receive error. Exiting.\n");
      return -1;
    }
    p = *((my_pkt *)t.payload);
    /* Verificam daca s-au trimis toate pachetele din fisier cu 
    confirmarea primirii mesajului de tip FINISH */
    if (p.type == FINISH) {
      break;
    }
  }
  printf("[SERVER] File transfer has ended.\n");
  close(fd);
  return 0;
}
