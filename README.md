# Sliding-Window-Protocol
Homework for the Communication Protocols course @ ACS, UPB 2019

# Algorithm

In rezolvarea temei, am folosit un protocol STOP AND WAIT. Astfel,
 transmitatorul trimite un mesaj cu date, asteapta confirmarea primirii
 mesajului de la receptor si trimite urmatorul mesaj. Receptorul asteapta
 mesaje cu date de la transmitator si trimite mesaje de confirmare. Legatura
 de date dintre transmitator si receptor este simulata prin intermediul 
 executabilului link. Confirmarile se transmit instantaneu. Implementarea contine
 urmatoarele detalii:
 - transmitatorul primeste ca argument numele fisierului care va fi transmis
 - receptorul nu cunoaste numele fisierului care va fi transmis si nici
 dimensiunea (aceste informatii trebuie primite de la transmitator)
 - transmitatorul trimite continutul fisierului catre receptor
 - receptorul asteapta datele de la transmitator si le scrie intr-un fisier.
 Am calculat proprietatiile principale care definesc o legatura de date:
 - viteza de transmisie - bandwidth (transmisa ca parametru)
 - timpul de propagare - delay (transmis ca parametru)
 Cantitatea de informatie aflata in zbor la un anumit momentde timp este bdp.
 bdp = bandwidth * delay * 1000

  Pasi de rezolvare:
 
 Fisierul send.c:
 1. Am stabilit tipurile de mesaje necesare (pentru confirmari ACK)
 2. Am trimis numele fisierului si dimensiunea acestuia, iar pentru fiecare 
 dintre acestea am asteptat ACK.
 3. Am implementat transmisia unui singur fisier, astfel am salvat mesajele
 intr-un buffer (vector de structuri), pe care le trimit cu ajutorul ferestrei
 glisante. (am citit din fisierul al carui continut trebuie sa-l trimit).
 4. Am realizat fereastra glisanta dupa cum urmeaza:
 - am calculat fereastra window ce reprezinta numarul maxim de cadre neconfirmate
 la orice moment de timp. 
 window = bdp / (FRAME_SIZE * BITS_NO) 
 - am realizat minim dintre window si numarul de pachete, daca exista mai puține
 pachete decât dimensiunea window, încearcă sa trimita pachete care nu exista
 - am pus pe fir un numar de cadre egal cu dimensiunea ferestrei, asteptand 
 confirmarile (primul for).
 - dupa ce primesc un ACK, pot trimite urmatorul cadru (asteptam confirmarile,
 deoarece pentru fiecare confirmare putem sa introducem un nou cadru in retea)
 (al doilea for). La fiecare ACK pentru cel mai vechi cadru trimis, fereastra
 gliseaza spre dreapta.
 - asteptam ACK-urile pentru celelalte window mesaje care au ramas pe fir
 (al treilea for). Verificam daca s-au trimis toate pachetele din fisier cu 
 confirmarea primirii mesajului de tip FINISH.

 Fisierul recv.c:
 1. Asteptam primirea mesajului cu numele fisierului. Dupa ce il primim, verificam
 daca este de tipul TYPE1, adica numele fisierului.
 2. Trimitem ACK pentru primirea numelui fisierului.
 3. Asteptam primirea mesajului cu dimensiunea fisierului. Verificam daca este
 de TYPE2, adica dimensiunea.
 4. Trimitem ACK pentru primirea dmensiunii fisierului.
 5. Cream fisierul in care scriem mesajul trimis de send.c.
 6. In while, asteptam primirea continutului fisierului de TYPE3, trimis de
 fereastra glisanta, pe care le scriem in fisierul creat cu noul nume. 
 (cat timp primim mesaje pana cand umplem fisierul cu dimensiunea primita la
 pas 3).
 7. La primirea fiecarui mesaj trimis de fereastra glisanta, trimitem ACK pentru
 confirmare.
  Pentru primele teste, fisierul receptionat este identic cu cel transmis.
