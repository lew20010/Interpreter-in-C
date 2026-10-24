#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
//Zeilenumbruch = 0x0a
//Leerzeichen = 0x20
int countBedingungen = 1;
typedef struct Variable {
    unsigned char name[32];
    int wert;
    int nenner;
    struct Variable *next;
} Variable;
typedef struct Bedingung{
    int posInListe;
    unsigned char elementOne[32];
    unsigned char elementTwo[32];
    unsigned char vergleichsOperator;
    unsigned char *data;
    bool schleife;
    struct Bedingung *next;

}Bedingung;
static Bedingung start;
static Variable variablen[1024];

static const unsigned char wortListe[13][4] = {
    {0x76, 0x61, 0x72, 0x20}, // var
    {0x61, 0x64, 0x64, 0x20}, // add
    {0x73, 0x75, 0x62, 0x20}, // sub
    {0x6D, 0x75, 0x6C, 0x20}, // mul
    {0x64, 0x69, 0x76, 0x20}, // div
    {0x64, 0x69, 0x76, 0x69}, //divi
    {0x6d, 0x6f, 0x64, 0x20}, //mod
    {0x73, 0x61, 0x79, 0x20}, //say
    {0x66, 0x6f, 0x72, 0x20}, //for
    {0x77, 0x65, 0x6e, 0x6e}, // wenn
    {0x61, 0x6e, 0x64, 0x20}, //and
    {0x6f, 0x64, 0x65, 0x72}, //oder
    {0x78, 0x6f, 0x72, 0x20} //xor
};

int hash(const unsigned char *name)
{
    unsigned long hashwert = 5381;
    int c;

    while ((c = (unsigned char)*name++) != '\0') {
        hashwert = hashwert * 33 + c;
    }
    hashwert = hashwert % 1024;
    int rueckgabe = (int)hashwert;
    return rueckgabe;
}
int countLines(const unsigned char *data, size_t size) {
    int rueckgabe = 0;

    for (size_t i = 0; i < size; i++) {
        if (data[i] == 0x0A) {
            rueckgabe++;
        }
    }

    printf("\n");
    return rueckgabe;
}

int appearsInWordList(unsigned char *data){
    //HIER ÄNDERN WENN WÖRTER HINZUGEFÜGT WERDEN
    for(int i = 0;i<13;i++){
        for(int y = 0; y<4; y++){
            if(data[y] == wortListe[i][y]){
                if(y==3){
                    return i;
                }
            } else {
                break;
            }
        }
    }
    return -1;
}
int wertAusWerte(unsigned char *werte, int anzahlStellen){
    int potenz = anzahlStellen-1;
    int rueckgabe = 0;
    for(int i=0; i < anzahlStellen; i++){
        rueckgabe = rueckgabe + (werte[i]-0x30)*pow(10,potenz);
        potenz--;

    }
    return rueckgabe;
}
int euklid(int zahlG, int zahlK){
    if(zahlG<0){
        zahlG = zahlG*(-1);
    }

    while(zahlG%zahlK != 0){
        int zwisch = zahlG;
        zahlG = zahlK;
        zahlK = zwisch%zahlK;
    }
    return zahlK;
}
void kuerzen(Variable *zeiger){
    int grosseZahl = zeiger->wert;
    int kleineZahl = zeiger->nenner;
    while(euklid(zeiger->wert, zeiger->nenner) != 1){
        int gcd = euklid(zeiger->wert, zeiger->nenner);
        zeiger->wert = zeiger->wert/gcd;
        zeiger->nenner = zeiger->nenner/gcd;
    }
}
void variableInitialisieren(unsigned char *data){
    int laengeNameVariable = 0;
    while(data[laengeNameVariable] != 0x20){
        laengeNameVariable++;
    }
    unsigned char nameVariable[32] = {'\0'};
    memcpy(nameVariable, data, laengeNameVariable);
    int hashwert = hash(nameVariable);
    Variable *current = &variablen[hashwert];
    while(current->next!=NULL){
        current = current->next;
    }
    Variable *neu = malloc(sizeof(Variable));
    memcpy(neu->name, nameVariable, sizeof(neu->name));
    neu->wert = 0;
    int minusZeichen = 0;
    laengeNameVariable = laengeNameVariable + 1;
    int ende = laengeNameVariable;
    if(data[ende]=='-'){
        ende = ende+1;
        laengeNameVariable = laengeNameVariable +1;
        minusZeichen = 1;
    }
    while(data[ende] != 0x0A && data[ende] != 0X20){
        ende++;
    }
    unsigned char *werte = malloc(ende-laengeNameVariable);
    memcpy(werte, data+laengeNameVariable, ende-laengeNameVariable);
    //Wenn ich zurückkehre hier den Wert aus werte extrahieren
    int value = wertAusWerte(werte,ende-laengeNameVariable);
    if(minusZeichen == 1){
        value = value *(-1);
    }
    neu->wert = value;
    neu->nenner = 1;
    neu->next = NULL;
    current->next = neu;
    free(werte);

}
int sindStringsGleich(unsigned char *stringOne, unsigned char *stringTwo){
    for(int i = 0; i<32;i++){
        if(stringOne[i] != stringTwo[i]){
            return 0;
        }
    }
    return 1;
}
void pointerMoven(Variable **zeigerWriteTo, Variable **zeigerReadOne, Variable **zeigerReadTwo,
unsigned char *nameVariableWriteTo, unsigned char *nameVariableReadOne, unsigned char *nameVariableReadTwo,
int laengeWriteTo, int laengeReadOne, int laengeReadTwo){
    bool writeToSuc = false;
    bool readOneSuc = false;
    bool readTwoSuc = false;


    while(zeigerWriteTo != NULL){
        unsigned char vergleichWT[32] = {'\0'};
        memcpy(vergleichWT, nameVariableWriteTo, laengeWriteTo); //Unnötig langsam aber es funktioniert
        if(sindStringsGleich((*zeigerWriteTo)->name, vergleichWT) == 1){
            writeToSuc = true;
            break;
        }
        *zeigerWriteTo = (*zeigerWriteTo)->next;
    }
    if(writeToSuc == false){
        unsigned char *fehlerWT = malloc(laengeWriteTo+1);
        memcpy(fehlerWT,nameVariableWriteTo, laengeWriteTo);
        fehlerWT[laengeWriteTo] = '\0';
        printf("Variable: %s", fehlerWT);
        printf(" nicht initialisiert.\n");
        free(fehlerWT);
        exit(1);
    }



    while(zeigerReadOne != NULL){
        unsigned char vergleichRO[32] = {'\0'};
        memcpy(vergleichRO, nameVariableReadOne, laengeReadOne);
        if(sindStringsGleich((*zeigerReadOne)->name, vergleichRO) == 1){
            readOneSuc = true;
            break;
        }
        *zeigerReadOne = (*zeigerReadOne)->next;
    }
    if(readOneSuc == false){
        unsigned char *fehlerRO = malloc(laengeReadOne+1);
        memcpy(fehlerRO,nameVariableReadOne, laengeReadOne);
        fehlerRO[laengeReadOne] = '\0';
        printf("Variable: %s", fehlerRO);
        printf(" nicht initialisiert.\n");
        free(fehlerRO);
        exit(1);
    }



    while(zeigerReadTwo != NULL){
        unsigned char vergleichRT[32] = {'\0'};
        memcpy(vergleichRT, nameVariableReadTwo, laengeReadTwo);
        if(sindStringsGleich((*zeigerReadTwo)->name, vergleichRT) == 1){
            readTwoSuc = true;
            break;
        }
        *zeigerReadTwo = (*zeigerReadTwo)->next;
    }
    if(readTwoSuc == false){
        unsigned char *fehlerRT = malloc(laengeReadTwo+1);
        memcpy(fehlerRT,nameVariableReadTwo, laengeReadTwo);
        fehlerRT[laengeReadTwo] = '\0';
        printf("Variable: %s", fehlerRT);
        printf(" nicht initialisiert.\n");
        free(fehlerRT);
        exit(1);
    }

}
void testPrintMethode(unsigned char *data){
    int count = 0;
    printf("hallo");
    while(data[count]!=';'){
        printf("Test: %c\n", data[count++]);
        if(count == 100000){
            printf("scheiße\n");
            break;
        }
    }
    if(data[count]==';'){
        printf("Test: %c\n", data[count++]);
    }
}
void addieren(unsigned char *data, bool sub){
    int laengeWriteTo = 0;
    int laengeReadOne = 0;
    int laengeReadTwo = 0;
    while(data[laengeWriteTo] != 0x20){
        laengeWriteTo = laengeWriteTo +1;
    }
    laengeReadOne = laengeWriteTo+1;
    while(data[laengeReadOne] != 0x20){
        laengeReadOne = laengeReadOne +1;
    }
    laengeReadTwo = laengeReadOne +1;
    while(data[laengeReadTwo] != 0x20 && data[laengeReadTwo] != 0x0A){
        laengeReadTwo = laengeReadTwo +1;
    }
    laengeReadTwo = laengeReadTwo-laengeReadOne-1;
    laengeReadOne = laengeReadOne-laengeWriteTo-1;

    unsigned char *nameVariableWriteTo = malloc(laengeWriteTo+1);
    unsigned char *nameVariableReadOne = malloc(laengeReadOne+1);
    unsigned char *nameVariableReadTwo = malloc(laengeReadTwo+1);
    memcpy(nameVariableWriteTo, data, laengeWriteTo);
    memcpy(nameVariableReadOne, data+laengeWriteTo+1, laengeReadOne);
    memcpy(nameVariableReadTwo, data+laengeWriteTo+laengeReadOne+2, laengeReadTwo);
    int hashWertWriteTo = hash(nameVariableWriteTo);
    int hashWertReadOne = hash(nameVariableReadOne);
    int hashWertReadTwo = hash(nameVariableReadTwo);


    Variable *zeigerWriteTo = &variablen[hashWertWriteTo];
    zeigerWriteTo = zeigerWriteTo->next;


    Variable *zeigerReadOne = &variablen[hashWertReadOne];
    zeigerReadOne = zeigerReadOne->next;


    Variable *zeigerReadTwo = &variablen[hashWertReadTwo];
    zeigerReadTwo = zeigerReadTwo->next;



    pointerMoven(&zeigerWriteTo, &zeigerReadOne, &zeigerReadTwo, nameVariableWriteTo, nameVariableReadOne, 
    nameVariableReadTwo, laengeWriteTo, laengeReadOne, laengeReadTwo);
    if(sub==false){
    zeigerWriteTo->wert = zeigerReadOne->wert * zeigerReadTwo->nenner + zeigerReadTwo->wert * zeigerReadOne->nenner;
    zeigerWriteTo->nenner = zeigerReadOne->nenner * zeigerReadTwo->nenner;
    }else{
    zeigerWriteTo->wert = zeigerReadOne->wert * zeigerReadTwo->nenner - zeigerReadTwo->wert * zeigerReadOne->nenner;
    zeigerWriteTo->nenner = zeigerReadOne->nenner * zeigerReadTwo->nenner;

    }
    free(nameVariableWriteTo);
    free(nameVariableReadOne);
    free(nameVariableReadTwo);
    kuerzen(zeigerWriteTo);
}

void printmachen(unsigned char *data){
    //Falls Wort
    if(data[0]==0x22){
        //falls zeilenumbruch
        if(data[1]==0x5c && data[2]==0x6e){
            printf("\n");
            return;
        }
        

        //laenge vom wort, dass geprintet wird tracken
        int laengeWort = 1;
        while(data[laengeWort++]!=0x22);
        laengeWort = laengeWort-2; //weil es 1 zu groß war
        unsigned char *ausgabe = malloc(laengeWort+1); //Um in das letzte Fach \0 zu tun
        memcpy(ausgabe, data+1, laengeWort);
        ausgabe[laengeWort] = '\0';
        printf("%s\n", ausgabe);
        free(ausgabe);

    }else{ // falls eine variabel geprintet wird

        int laengeWort = 1;
        while(data[laengeWort]!=0x20 && data[laengeWort] != 0x0A){
            laengeWort++;
        }
        if(laengeWort>31){
            printf("Variable laenger als 31 Zeichen\n");
            exit(1);
        }

        unsigned char *variablename = malloc(laengeWort);
        memcpy(variablename, data, laengeWort);
        int hashwert = hash(variablename); //kritische stelle

        Variable *current = &variablen[hashwert];
        current = current->next;

        if(current != NULL){
            bool found = false;
            unsigned char vergleich[32] = {'\0'};
            memcpy(vergleich, variablename, laengeWort);
            free(variablename);
            while(current != NULL){
                if(sindStringsGleich(current->name, vergleich) == 1){
                    found = true;
                    
                }
                if(found == true){
                    float ausgabe = ((current->wert)*1.0)/((current->nenner)*1.0);
                    if(ausgabe - ((int)ausgabe) == 0){
                        printf("%d\n", (int)ausgabe);
                    }else{
                        printf("%f\n", ausgabe);
                    }
                    break;
                }
            }
            
        }else{
            unsigned char *fehlermeldung = malloc(laengeWort+1);
            memcpy(fehlermeldung, variablename, laengeWort);
            fehlermeldung[laengeWort] = '\0';
            printf("Folgende Variable existiert nicht: %s\n", fehlermeldung);
            free(fehlermeldung);
            exit(1);
        }

        

        

    }


}

void divisionKomma(unsigned char *data, bool integer){
    int laengeWriteTo = 0;
    int laengeReadOne = 0;
    int laengeReadTwo = 0;
    while(data[laengeWriteTo] != 0x20){
        laengeWriteTo = laengeWriteTo +1;
    }
    laengeReadOne = laengeWriteTo+1;
    while(data[laengeReadOne] != 0x20){
        laengeReadOne = laengeReadOne +1;
    }
    laengeReadTwo = laengeReadOne +1;
    while(data[laengeReadTwo] != 0x20 && data[laengeReadTwo] != 0x0A){
        laengeReadTwo = laengeReadTwo +1;
    }
    laengeReadTwo = laengeReadTwo-laengeReadOne-1;
    laengeReadOne = laengeReadOne-laengeWriteTo-1;

    unsigned char *nameVariableWriteTo = malloc(laengeWriteTo+1);
    unsigned char *nameVariableReadOne = malloc(laengeReadOne+1);
    unsigned char *nameVariableReadTwo = malloc(laengeReadTwo+1);
    memcpy(nameVariableWriteTo, data, laengeWriteTo);
    memcpy(nameVariableReadOne, data+laengeWriteTo+1, laengeReadOne);
    memcpy(nameVariableReadTwo, data+laengeWriteTo+laengeReadOne+2, laengeReadTwo);
    int hashWertWriteTo = hash(nameVariableWriteTo);
    int hashWertReadOne = hash(nameVariableReadOne);
    int hashWertReadTwo = hash(nameVariableReadTwo);
    Variable *zeigerWriteTo = &variablen[hashWertWriteTo];
    zeigerWriteTo = zeigerWriteTo->next;
    Variable *zeigerReadOne = &variablen[hashWertReadOne];
    zeigerReadOne = zeigerReadOne->next;
    Variable *zeigerReadTwo = &variablen[hashWertReadTwo];
    zeigerReadTwo = zeigerReadTwo->next;
    bool writeToSuc = false;
    bool readOneSuc = false;
    bool readTwoSuc = false;


    while(zeigerWriteTo != NULL){
        unsigned char vergleichWT[32] = {'\0'};
        memcpy(vergleichWT, nameVariableWriteTo, laengeWriteTo); //Unnötig langsam aber es funktioniert
        if(sindStringsGleich(zeigerWriteTo->name, vergleichWT) == 1){
            writeToSuc = true;
            break;
        }
        zeigerWriteTo = zeigerWriteTo->next;
    }
    if(writeToSuc == false){
        unsigned char *fehlerWT = malloc(laengeWriteTo+1);
        memcpy(fehlerWT,nameVariableWriteTo, laengeWriteTo);
        fehlerWT[laengeWriteTo] = '\0';
        printf("Variable: %s", fehlerWT);
        printf(" nicht initialisiert.\n");
        exit(1);
    }



    while(zeigerReadOne != NULL){
        unsigned char vergleichRO[32] = {'\0'};
        memcpy(vergleichRO, nameVariableReadOne, laengeReadOne);
        if(sindStringsGleich(zeigerReadOne->name, vergleichRO) == 1){
            readOneSuc = true;
            break;
        }
        zeigerReadOne = zeigerReadOne->next;
    }
    if(readOneSuc == false){
        unsigned char *fehlerRO = malloc(laengeReadOne+1);
        memcpy(fehlerRO,nameVariableReadOne, laengeReadOne);
        fehlerRO[laengeReadOne] = '\0';
        printf("Variable: %s", fehlerRO);
        printf(" nicht initialisiert.\n");
        exit(1);
    }



    while(zeigerReadTwo != NULL){
        unsigned char vergleichRT[32] = {'\0'};
        memcpy(vergleichRT, nameVariableReadTwo, laengeReadTwo);
        if(sindStringsGleich(zeigerReadTwo->name, vergleichRT) == 1){
            readTwoSuc = true;
            break;
        }
        zeigerReadTwo = zeigerReadTwo->next;
    }
    if(readTwoSuc == false){
        unsigned char *fehlerRT = malloc(laengeReadTwo+1);
        memcpy(fehlerRT,nameVariableReadTwo, laengeReadTwo);
        fehlerRT[laengeReadTwo] = '\0';
        printf("Variable: %s", fehlerRT);
        printf(" nicht initialisiert.\n");
        exit(1);
    }

    if(zeigerReadTwo->wert == 0){
        printf("Nulldivision nicht moeglich!\n");
        exit(1);
    }
    if(integer==false){
    zeigerWriteTo->wert = zeigerReadOne->wert * zeigerReadTwo->nenner;
    zeigerWriteTo->nenner = zeigerReadTwo->wert * zeigerReadOne->nenner;
    kuerzen(zeigerWriteTo);
    }else{
        zeigerWriteTo->wert= zeigerReadOne->wert/zeigerReadTwo->wert;
    }

    free(nameVariableWriteTo);
    free(nameVariableReadOne);
    free(nameVariableReadTwo);



}
unsigned char *zeileAusfuehren(unsigned char *data, bool last);

int variableWertExtrahieren(unsigned char *nameElemen){
    unsigned char nameElement[32] = {'\0'};
    memcpy(nameElement, nameElemen, 31);
    Variable *current = &variablen[hash(nameElement)];
    current = current->next;
    bool found = false;
    while(found == false){
        if(current != NULL){
            if(sindStringsGleich(nameElement, current->name)==1){
                found = true;
            }else{
                current = current->next;
            }
        }else{
            break;
        }
    }
    if(found == false){
        printf("Variable existiert nicht\n");
        exit(0);
    }
    return current->wert;
}
void moduloRechnen(unsigned char *data){
    int laengeWriteTo = 0;
    int laengeReadOne = 0;
    int laengeReadTwo = 0;
    while(data[laengeWriteTo] != 0x20){
        laengeWriteTo = laengeWriteTo +1;
    }
    laengeReadOne = laengeWriteTo+1;
    while(data[laengeReadOne] != 0x20){
        laengeReadOne = laengeReadOne +1;
    }
    laengeReadTwo = laengeReadOne +1;
    while(data[laengeReadTwo] != 0x20 && data[laengeReadTwo] != 0x0A){
        laengeReadTwo = laengeReadTwo +1;
    }
    laengeReadTwo = laengeReadTwo-laengeReadOne-1;
    laengeReadOne = laengeReadOne-laengeWriteTo-1;

    unsigned char *nameVariableWriteTo = malloc(laengeWriteTo+1);
    unsigned char *nameVariableReadOne = malloc(laengeReadOne+1);
    unsigned char *nameVariableReadTwo = malloc(laengeReadTwo+1);
    memcpy(nameVariableWriteTo, data, laengeWriteTo);
    memcpy(nameVariableReadOne, data+laengeWriteTo+1, laengeReadOne);
    memcpy(nameVariableReadTwo, data+laengeWriteTo+laengeReadOne+2, laengeReadTwo);
    int hashWertWriteTo = hash(nameVariableWriteTo);
    int hashWertReadOne = hash(nameVariableReadOne);
    int hashWertReadTwo = hash(nameVariableReadTwo);

    Variable *zeigerWriteTo = &variablen[hashWertWriteTo];
    zeigerWriteTo = zeigerWriteTo->next;


    Variable *zeigerReadOne = &variablen[hashWertReadOne];
    zeigerReadOne = zeigerReadOne->next;


    Variable *zeigerReadTwo = &variablen[hashWertReadTwo];
    zeigerReadTwo = zeigerReadTwo->next;


    pointerMoven(&zeigerWriteTo, &zeigerReadOne, &zeigerReadTwo, nameVariableWriteTo, nameVariableReadOne, 
    nameVariableReadTwo, laengeWriteTo, laengeReadOne, laengeReadTwo);
    if(zeigerReadOne->nenner == 1 && zeigerReadTwo->nenner == 1){
    zeigerWriteTo->wert = zeigerReadOne->wert % zeigerReadTwo->wert;
    }else{
        printf("Man kann keine nicht ganzen Zahlen Modulo rechnen!\n");
        exit(1);
    }

    free(nameVariableWriteTo);
    free(nameVariableReadOne);
    free(nameVariableReadTwo);
    kuerzen(zeigerWriteTo);
}
void multiplizieren(unsigned char *data){
    int laengeWriteTo = 0;
    int laengeReadOne = 0;
    int laengeReadTwo = 0;
    while(data[laengeWriteTo] != 0x20){
        laengeWriteTo = laengeWriteTo +1;
    }
    laengeReadOne = laengeWriteTo+1;
    while(data[laengeReadOne] != 0x20){
        laengeReadOne = laengeReadOne +1;
    }
    laengeReadTwo = laengeReadOne +1;
    while(data[laengeReadTwo] != 0x20 && data[laengeReadTwo] != 0x0A){
        laengeReadTwo = laengeReadTwo +1;
    }
    laengeReadTwo = laengeReadTwo-laengeReadOne-1;
    laengeReadOne = laengeReadOne-laengeWriteTo-1;

    unsigned char *nameVariableWriteTo = malloc(laengeWriteTo+1);
    unsigned char *nameVariableReadOne = malloc(laengeReadOne+1);
    unsigned char *nameVariableReadTwo = malloc(laengeReadTwo+1);
    memcpy(nameVariableWriteTo, data, laengeWriteTo);
    memcpy(nameVariableReadOne, data+laengeWriteTo+1, laengeReadOne);
    memcpy(nameVariableReadTwo, data+laengeWriteTo+laengeReadOne+2, laengeReadTwo);
    int hashWertWriteTo = hash(nameVariableWriteTo);
    int hashWertReadOne = hash(nameVariableReadOne);
    int hashWertReadTwo = hash(nameVariableReadTwo);


    Variable *zeigerWriteTo = &variablen[hashWertWriteTo];
    zeigerWriteTo = zeigerWriteTo->next;


    Variable *zeigerReadOne = &variablen[hashWertReadOne];
    zeigerReadOne = zeigerReadOne->next;


    Variable *zeigerReadTwo = &variablen[hashWertReadTwo];
    zeigerReadTwo = zeigerReadTwo->next;



    pointerMoven(&zeigerWriteTo, &zeigerReadOne, &zeigerReadTwo, nameVariableWriteTo, nameVariableReadOne, 
    nameVariableReadTwo, laengeWriteTo, laengeReadOne, laengeReadTwo);
    zeigerWriteTo->wert = zeigerReadOne->wert * zeigerReadTwo->wert;
    zeigerWriteTo->nenner = zeigerReadOne->nenner * zeigerReadTwo->nenner;
    
    free(nameVariableWriteTo);
    free(nameVariableReadOne);
    free(nameVariableReadTwo);
    kuerzen(zeigerWriteTo);
}
bool pruefeAussage(unsigned char *elementOne, unsigned char *elementTwo, unsigned char vergleichsOperator){
    int elementOneWert = variableWertExtrahieren(elementOne);
    int elementTwoWert = variableWertExtrahieren(elementTwo);

    if(vergleichsOperator == '='){
        return (elementOneWert == elementTwoWert);
    }else{
        if(vergleichsOperator == '>'){
            return (elementOneWert > elementTwoWert);
        }else{
            return (elementOneWert < elementTwoWert);
        }
    }
    return false;
}
void wennAusfuehren(unsigned char *data){
    //Zahl extrahieren
    //testPrintMethode(data);
    //printf("Hallo\n");
    int laengeIndex = 0;
    while(data[laengeIndex] !=0x20 && data[laengeIndex] != 0x0A){
        laengeIndex++;
    }
    unsigned char *indexInChar = malloc(laengeIndex);
    memcpy(indexInChar, data, laengeIndex);
    int indexInInt = 0;
    for(int i = 0; i<laengeIndex; i++){
        indexInInt = indexInInt + (data[i]-0x30)* (int)pow(10.0, i+laengeIndex-1);
    }
    free(indexInChar);
    

    //Den Block aus den faten suchen
    Bedingung *current = &start;
    for(int i = 0; i<indexInInt; i++){
        current = current->next;
    }
    size_t groesseData = 0;
    while(current->data[groesseData] != ';'){
        groesseData++;
    }
    groesseData++;

    int anzahlNeueZeilen = countLines(current->data, groesseData);
    //Sorgen dass der forBlock Nur ausgefuehrt wird wenn die aussage wahr ist
    bool aussageWahr = pruefeAussage(current->elementOne, current->elementTwo, current->vergleichsOperator);
    // Werte extrahieren
    //printf("Element 1 = %d\n", elementOneWert);
    if(current->schleife == false && aussageWahr == true){
        int laengeDaten = 0;
        while(current->data[laengeDaten] != ';'){
            laengeDaten++;
        }
        unsigned char *datenFuerArbeit = malloc(laengeDaten);
        memcpy(datenFuerArbeit, current->data, laengeDaten);
        for(int c = 0; c<=anzahlNeueZeilen; c++){
            if(c==anzahlNeueZeilen && datenFuerArbeit != NULL){
                datenFuerArbeit = zeileAusfuehren(datenFuerArbeit, true);
            }else{
            if(datenFuerArbeit != NULL){
                datenFuerArbeit = zeileAusfuehren(datenFuerArbeit, false);
            }

            }
            //testPrintMethode(datenFuerArbeit);
        }
        free(datenFuerArbeit);
}


    if(current->schleife == true){
        size_t laengeData = 0;
        while(current->data[laengeData] != ';'){
            laengeData++;
        }
        laengeData++;
        while(pruefeAussage(current->elementOne, current->elementTwo, current->vergleichsOperator)){
            unsigned char *datenMitDenenWirArbeiten = malloc(laengeData);
            memcpy(datenMitDenenWirArbeiten, current->data, laengeData);
            for(int c = 0; c<=anzahlNeueZeilen; c++){
                if(c==anzahlNeueZeilen && datenMitDenenWirArbeiten != NULL){
                    datenMitDenenWirArbeiten = zeileAusfuehren(datenMitDenenWirArbeiten, true);
                }else{
                if(datenMitDenenWirArbeiten != NULL){
                    datenMitDenenWirArbeiten = zeileAusfuehren(datenMitDenenWirArbeiten, false);
                }

                }
            }
            free(datenMitDenenWirArbeiten);}
            
}






}


unsigned char *zeileAusfuehren(unsigned char *data, bool last){
    int laengeZeile = 0;
    bool ende = false;
    while(!ende){
        if(data[laengeZeile] != 0x0A){
            laengeZeile +=1;
        }else{
            ende = true;
            if(laengeZeile == 0){
                return &data[1];
            }
        }
    }
    if(data[laengeZeile+1] == 0){
        data[laengeZeile+1] = ';'; // ich weiß nicht warum das überhaupt nicht der fall sein wird, aber es kam ein fehler auf
    }
    unsigned char *currentWorkWith = malloc(laengeZeile);
    memcpy(currentWorkWith, data, laengeZeile);
    int coordinates = -1;
    if(laengeZeile >= 4){
        coordinates = appearsInWordList(currentWorkWith);
    
}

//Methoden aufrufen für die einzelnen Aktionen


int wert = 0; //falls vierzeichen Wort
if(coordinates==9 || coordinates==5){
    wert++;
}
//Fall Variable erstellen
if(coordinates == 0){
    variableInitialisieren(data+4);
}

//Falls Say
if(coordinates == 7){
    printmachen(data+4);
}
//falls div
if(coordinates == 4){
    divisionKomma(data+4, false);
}
//falls divi
if(coordinates == 5){
    divisionKomma(data+5, true);
}
//falls add
if(coordinates == 1){
    addieren(data+4,false);
}
//falls sub
if(coordinates == 2){
    addieren(data+4, true);
}
//falls mull
if(coordinates == 3){
    multiplizieren(data+4);
}
//falls wenn oder loop
if(coordinates == 9){
    //testPrintMethode(data);
    wennAusfuehren(data+5);
}
//falls mod
if(coordinates == 6){
    moduloRechnen(data+4);
}
free(currentWorkWith);

if(last==false && data[laengeZeile +1] != ';'){
return &data[laengeZeile+1];
}   
return NULL;

}
void zahlInChar(unsigned char *c, int n, int anzStellen){
    int speicher = n;
    int currFach = 0;
    for(int i = anzStellen-1; i>=0; i--){
        int zifferAnStelle = (int)(speicher/pow(10.0,i*1.0));
        c[currFach++] = zifferAnStelle + 0x30;
        speicher = speicher - zifferAnStelle*(int)pow(10.0,i*1.0);
    }
}
int wieLang(unsigned char *data){
    int count = 0;
    while(data[count] != ';'){
        count = count+1;
    }
    return count +1;
}
int komprimiere(unsigned char*data, size_t groesse){
    //finde die Wenns
    int pos;
    size_t laengeAbschnittInsgesamt = 0;
    bool ende = false;
    while(ende == false){
        if(data[0]==';'){
            break;
        }
        if((data[0]=='w'&&data[1]=='e'&& data[2]=='n'&&data[3]=='n') || (data[0]=='l'&&data[1]=='o'&& data[2]=='o'&&data[3]=='p')){
            bool istschleife = (data[0] == 'l');
            if(istschleife){
                data[0] = 'w';
                data[1] = 'e';
                data[2] = 'n';
                data[3] = 'n';
            }
            laengeAbschnittInsgesamt = 0;
            unsigned char *posStart = data;
            data = data +5;
            int variableOneLaenge = 0;
            while(data[variableOneLaenge] != 0x20 && data[variableOneLaenge] != '=' && data[variableOneLaenge] != '>' && data[variableOneLaenge] != '<'){
                variableOneLaenge++;
            }
            unsigned char *variableOneName = malloc(variableOneLaenge+1);
            memcpy(variableOneName, data, variableOneLaenge);
            variableOneName[variableOneLaenge] = '\0';
            data = data + variableOneLaenge;
            int skipOne = 0;
            int skipTwo = 0;
            if(data[0]==0x20){
                data = data +1;
                skipOne = 1;
            }
            unsigned char vergleichsoperator = data[0];
            data = data+1;
            if(data[0]== 0x20){
                data = data+1;
                skipTwo = 1;
            }


            int variableTwoLaenge = 0;
            while(data[variableTwoLaenge] != '{'){
                variableTwoLaenge++;
            }
            unsigned char *variableTwoName = malloc(variableTwoLaenge+1);
            memcpy(variableTwoName, data, variableTwoLaenge);
            variableTwoName[variableTwoLaenge] = '\0';

            //jetzt kuemmern wir uns darum den Block einzugrenzen
            data = data+variableTwoLaenge;
            laengeAbschnittInsgesamt = 5 + variableOneLaenge + skipOne + 1 +skipTwo +variableTwoLaenge+1;
            int klammernCounter = 1;
            bool anfang = true;
            int countInhalt = 0;
            unsigned char *aussage = data+1;
            while(klammernCounter != 0){
                countInhalt++;
                if(data[0]=='{' && anfang == false){
                    klammernCounter = klammernCounter + 1;
                }
                if(data[0]=='}'){
                    klammernCounter = klammernCounter - 1;
                }
                data = data+1;
                anfang = false;
                laengeAbschnittInsgesamt = laengeAbschnittInsgesamt+1;
            }
            countInhalt = countInhalt-2;

            unsigned char *dataBedingung = malloc(countInhalt+1); // Fuer den struct und +1, da ; ans ende muss
            memcpy(dataBedingung, aussage+1, countInhalt);
            dataBedingung[countInhalt-1] = 0x0A;
            dataBedingung[countInhalt] = ';';
            Bedingung *curr = &start;
            for(int i = 1; i<countBedingungen; i++){
                curr = curr->next;
                 //testPrintMethode(dataBedingung);
                 //printf("countBedingungen %d\n", countBedingungen);
            }
            //printf("count Bedingungern %d\n", countBedingungen);
            //Kurze exkursion um an die variabelnwerte zu kommen


            Bedingung *neu = malloc(sizeof(Bedingung));
            neu->posInListe = countBedingungen;
            memcpy(neu->elementOne, variableOneName, variableOneLaenge);
            memcpy(neu->elementTwo, variableTwoName, variableTwoLaenge);
            free(variableOneName);
            free(variableTwoName);
            neu->vergleichsOperator = vergleichsoperator;
            neu->schleife = istschleife;
            neu->data = dataBedingung;
            countBedingungen++;
            //testPrintMethode(neu->data);
            curr->next = neu;
            int x = komprimiere(neu->data, countInhalt+1);
            //testPrintMethode(neu->data);



            


            while(data[0]!=0x0A){
                data = data+1;
                laengeAbschnittInsgesamt= laengeAbschnittInsgesamt+1;
            }
            data = data+1;
            int laengeZahl = 1;
            laengeZahl = (int)(log10(neu->posInListe))+1;
            
            unsigned char *stelleBedingungenListe = malloc(laengeZahl);
            zahlInChar(stelleBedingungenListe, neu->posInListe, laengeZahl);
            for(int i = 0; i<laengeZahl; i++){
                posStart[5+i] = stelleBedingungenListe[i];
            }
            free(stelleBedingungenListe);
            posStart[5+laengeZahl] = 0x0A;
            int laengeBisEnde = 0;
            laengeBisEnde = wieLang(data);
            memcpy(posStart+6+laengeZahl, data, laengeBisEnde);
            data = posStart +6 +laengeZahl;
        
        }else{
        while(data[0]!=0x0A){
            data = data+1;
        }
        data = data+1;
        if(data[0]==0x3b){
            return 1;
        }
    }
    }
    return laengeAbschnittInsgesamt;
}
void testPrintDataLinkedListe(){
    Bedingung *curr = &start;
    curr = curr->next;
    printf("curr %c\n", curr->vergleichsOperator);
    int zaehler = 1;
    while(curr != NULL){
        printf("Zaehler: %d\n", zaehler++);
        //printf("WASPASSIERTHIE %d\n", curr->posInListe);
        testPrintMethode(curr->data);
        curr = curr->next;
    }
    //printf("count Bedingungen %d\n", countBedingungen);
}
int main(void) {
    clock_t beginn = clock();
    char pfad[1024];

    printf("Dateipfad eingeben: ");

    if (fgets(pfad, sizeof(pfad), stdin) == NULL) {
        return 1;
    }

    pfad[strcspn(pfad, "\r\n")] = '\0';

    FILE *datei = fopen(pfad, "rb");
    if (datei == NULL) {
        perror("Datei konnte nicht geöffnet werden");
        return 1;
    }

    if (fseek(datei, 0, SEEK_END) != 0) {
        fclose(datei);
        return 1;
    }

    long dateigroesse = ftell(datei);

    if (dateigroesse < 0 ||
        fseek(datei, 0, SEEK_SET) != 0) {
        fclose(datei);
        return 1;
    }

    size_t groesse = (size_t)dateigroesse;

    unsigned char *daten = malloc(groesse + 1);

    if (daten == NULL) {
        fclose(datei);
        perror("Speicherreservierung fehlgeschlagen");
        return 1;
    }

    size_t gelesen = fread(daten, 1, groesse, datei);
    fclose(datei);

    if (gelesen != groesse) {
        free(daten);
        fprintf(stderr, "Fehler beim Lesen der Datei.\n");
        return 1;
    }

    daten[groesse] = '\0';

    printf("Datei erfolgreich geladen: %zu Bytes\n", groesse);
    komprimiere(daten, groesse);
    //testPrintMethode(daten);
    //testPrintDataLinkedListe();
    countBedingungen = countBedingungen-1; //weil ich das mit dem rekursiven Aufruf nicht richtig koordiniert habe, ups
    int anzahlNeueZeilen = countLines(daten, groesse);
    for(int c = 0; c<=anzahlNeueZeilen; c++){
        if(c==anzahlNeueZeilen && daten != NULL){
            daten = zeileAusfuehren(daten, true);
        }else{
        if(daten != NULL){
            daten = zeileAusfuehren(daten, false);
        }

        }
    }
    free(daten);
    clock_t ende = clock();

    double sekunden = (double)(ende - beginn) / CLOCKS_PER_SEC;

    printf("Ausführungszeit: %.6f Sekunden\n", sekunden);
    return 0;
}

