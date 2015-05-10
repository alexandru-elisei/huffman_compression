Structuri de Date - TEMA 2
ELISEI ALEXANDRU
316CC


Cuprins:
	1. COMPILARE
	2. COMENZI BONUS
	3. DESCRIERE
	4. DETALII IMPLEMENTARE


1. COMPILARE

Programul este compilat cu comanda "make build (sau "make" simplu). La compilare
se poate modifica variabila CFLAGS ("make CFLAGS='-DDEBUG=1 -g'") si astfel pot
fi folosite macro-urile de debugging definite in common.h (DEBINFO si DEBMSG).

Fisierele rezultate sunt sterse cu comanda "make clean".


2. COMENZI BONUS

Am implementat si operatiunea inversa, de decomprimare.


3. DESCRIERE

#Se citesc din fisierul de intrare optiunea de a folosi stiva sa coada si
#liniile hard diskului in main.c. Aici se genereaza hard disk-ul si, in functie
#de optiune, stiva sau coada. Intr-o bucla infinita se citesc restul de
#instructiuni din fisier, rand cu rand, iar citirea se termina la intalnirea 
#instructiunii de terminare ("::e"). La expirarea timpului alocat unei 
#instructiuni se citeste urmatoarea instructiune din fisier si se adauga 
#stivei/cozii de comenzi.
#
#La fiecare pas de timp se executa instructiunea curenta, daca avem instructiuni
#de executat, altfel se asteapta pe pozitia curenta expirarea timpului si citirea
#unei noi instructiuni.
#
#In cazul stivei, ultima instructiune citita este prioritara (mereu se incearca
#executarea instructiunii din capul stivei), iar in cazul cozii prima
#instructiune citita si neexecutata este prioritare (se incearca executarea
#instructiunii din head).
#
#Executarea unei instructiunii are un damage asupra hard disk-ului, precum si
#asteptarea pe un sector in cazul in care nu avem instructiuni de executat.
#
#La primirea instructiunii de terminare ("::e"), comportarea programului e
#diferita in functie de optiunea selectata:
#- in cazul cozii, deoarece comenzile prioritare sunt cele citite primele, se
#  executa toate comanzile neexecutate pana atunci, apoi se iese din program.
#- in cazul stivei, doarece comenzile prioritare sunt cele citite ultimele, se
#  iese din program fara a se executa restul de comenzi.
#
#La iesirea din program se tipareste damage mediu asupra hard disk-ului, impartit
#in patru zone.


4. DETALII IMPLEMENTARE

#a) main.c
#
#Aici sunt declarate coada (prin variabilele "head" si "tail") si stiva (prin
#variabila "top"). Aici este parcurs fisierul de comenzi si implementata executia
#lor conform descrierii functionarii programului.
#
#Functiile au fost concepute de asa natura incat sa aiba ca return code o
#variabila a carei semnificatie este implementata in common.h prin intermediul 
#enumerarii hdd_result. Verificarea codului returnat de o functie se face cu
#macro-ul CHECK_RESULT care iese din program la intalnirea unui cod returnat de
#o functie, cod diferit de HDD_SUCCESS sau HDD_SEEK_INCOMPLETE. Codul returnat
#la terminarea programului este identic cu codul din enumerare, si, pe deasupra,
#se incearca printarea mesajului corespunzator erorii, eroarea putand fi astfel 
#interpretata de utilizator. In cazul functionarii anormale a programului, in
#interiorul macro-ului, se incearca salvarea datelor in fisierul de iesire si
#eliberarea memoriei alocate pana atunci.
#
#Dupa tiparirea damage-ului, se elibereaza memoria ocupata de hard disk, de
#cursor si de buffer-ul de citire din fisierul de intrare.
#
#b) common.h si common.c
#
#Fisierul common.h contine constantele folosite de alte fisierele ale
#programului, enumerarea hdd_result care contine return codes asteptate de la
#functii, structura hdd_address care stocheaza o adresa a unui sector pe hard,
#exprimata prin linie si index, functia de printare a mesajelor asociate
#enumerarii hdd_result si macro-urile folosite pentru debugging. DEBMSG tipareste
#un mesaj, precum si variabila ce contine mesajul, si DEBINFO tipareste expresia
#de evaluat precum si rezultatul evaluarii ei. Ambele tiparesc fisierul, linie 
#si functia de unde au fost apelate prin variabilele de compilator __FILE__, 
#__LINE__ si __FUNCTION__. Macro-urile se activeaza explicit prin compilarea cu 
#definirea constantei DEBUG (gcc -DDEBUG=1 <input> <output>).
#
#In common.c se gaseste functia care tipareste mesajele asociate codurilor
#returnate de functii.
#
#c) hdd.h si hdd.c
#
#Fisierul hdd.h contine functiile publice (vizibile in alte fisiere) asociate
#operarii cu hard disk-ul, precum si structura acestuia (struct hdd_sector) si
#a capului de citire (struct hdd_head).
#
#Hard disk-ul a fost implementat ca o lista neliniara cu pointeri catre elementul
#urmator in linie ("next"), cel de deasupra ("above") si cel de dedesubt
#("below"), pointeri care au o valoare diferita de NULL pentru elementele de pe
#linie cu indexul 0. Cautarea pe disk se poate face incrememtal, element cu
#element in interiorul unei linii, iar la elementele de index 0 care reprezinta
#legatura cu listele superioare/inferioare se poate trece la liniile adiacente.
#
#d) queue.h si queue.c
#
#Fisierul header queue.h contine functiile publice asociate cozii de comenzi,
#precum si structura acesteia. Coada functioneaza pe principiul first in - first
#out. Comenzile se transmit prin intermediul function cq_enqueue sub forma liniei
#citite din fisierul de intrare, se interpreteaza si se salveaza ca succesor al
#variabilei tail. Comenzile executate sunt cele din variabila head, primele
#introduse, si dupa executie acestea se sterg, variabila head mutandu-se pe
#urmatoarea comanda citita.
#
#Coada este accesata in main.c prin intermediul variabilelor "head" - contine
#prima comanda citita, adica cea care este prioritara, si "tail" - ultima comanda
#citita, unde se face introducere de noi comenzi.
#
#Comenzile multi-read si multi-write sunt transformate in comenzile elementare si
#adaugate normal la coada. Acestea functioneaza indiferent de configuratia hard
#diskului: daca se ajunge pe ultima linie se continua comanda spre linia 0, daca
#se ajunge pe linia 0 se continua comanda spre ultima linie. Daca hard diskul are
#doar o linie, se repeta comanda pe acea linie.
#
#e) stack.h si stack.c
#
#Fisierul header stack.h contine functiile publice asociate stivei de comenzi,
#precum si structura acesteia. Coada functioneaza pe principiul last in - first
#out si aste accesata in main.c prin intermediul variabilei top care reprezinta
#ultima comanda salva/prima ce se va executa.
#
#Adaugarea de comenzii se face cu functia cs_push, care primeste ca parametru
#linia citita din fisierul de intrare, care va fi intrepretata si salvata. La
#executia unei comenzi, variabile top se muta pe penultima comanda citita.
#
#Comenzile multi-read si multi-write sunt transformate in comenzile elementare si
#adaugate la stiva prin intermediul unei stive temporare, astfel se asigura ca
#prima comanda ce se vrea efectuata este prima comanda efectuta (ultima adaugata
#in stiva). Acestea functioneaza indiferent de configuratia hard disk-ului: daca
#se ajunge pe ultima linie se continua comanda spre linia 0, daca se ajunge pe 
#linia 0 se continua comanda spre ultima linie. Daca hard diskul are doar o 
#linie, se repeta comanda pe acea linie.
