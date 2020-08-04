#include <stdio.h>
#include <stdlib.h>
#include <string.h>



struct InfoBlock {
  unsigned int RouterID;
  unsigned char Flag;
  int visited;
  int counter; //bruker en teller her for å vite hvor mange koblinger hver ruter har.
  unsigned char Pointers[10]; //pupuleres med en int som representerer ruterID. Da kan man enkelt hente den ruteren med routerStruct[routerID]
  char Model[];
}*routerStruct[0];

struct InfoBlock *ptrStruct;


int read(FILE *file) {
  unsigned int numb;
  fread(&numb, 4, 1, file); //finner antall rutere
  //printf("numb = %d\n", numb);


  int i;
  for(i = 0; i < numb; i++) {
    routerStruct[i] = (struct InfoBlock*) malloc(sizeof(struct InfoBlock) + 9); //aner ikke hvorfor men måtte legge til 9 i mallocen for at det skulle funke
    memset(routerStruct[i], 0, sizeof(struct InfoBlock));

    unsigned int routerID;
    fread(&routerID, 4, 1, file); //henter inn routerID som vi vet er 4 byte
    routerStruct[i] -> RouterID = routerID; //lagrer det i structen
    // printf("RuterID = %d\n", routerID);

    unsigned char Flag;
    Flag = fgetc(file); //kunne brukt fread her, men gidder ikke endre på det etter jeg testet forskjellige ting
    routerStruct[i] -> Flag = Flag;
    // int converted = decToBin(Flag);
    // printf("%d\n", converted);
    // printf("Flagg i dec = %d\n", Flag);

    unsigned char length;
    length = fgetc(file);
    //printf("Lengde i dec = %d\n", length);

    char Model[length];
    fread(Model, length + 1, 1, file); //leser inn modell/produsent med lengde pluss en fordi da får jeg med den siste nullbyten. Kunne brukt en ekstra fgetc, men gidder ikke endre på det

    strcpy(routerStruct[i] -> Model, Model);
    routerStruct[i] -> counter = 0;

    //printf("Modell = %s\n\n", Model);

    // printf("%d\n", routerStruct[i] -> RouterID);
    // printf("%d\n", routerStruct[i] -> Flag);
    // printf("%s\n\n", routerStruct[i] -> Model);
  }

  int k;
  int l;
  unsigned char nullRuter = NULL; //hvis jeg ikke gjør det på denne måten så kan jeg ikke få inn de som peker eller blir pekt på av ruter 0.
  unsigned char ptr = &nullRuter;
  for (k = 0; k < numb; k++) {
    for (l = 0; l < 10; l++) {
      routerStruct[k] -> Pointers[l] = ptr; //pupulerer arrayet med like nullpekere så jeg kan sammenligne i while-løkka nedenfor
    }
  }

  while(1) { //vurderte å bruke !feof(file) her, men da får man siste verdi to ganger så brukte heller break før jeg la de inn i structen
    int connR1;
    int connR2;
    fread(&connR1, 4, 1, file); //henter ut ruterne for koblingen her og legger det i sine verdier
    fread(&connR2, 4, 1, file);
    fgetc(file); //
    if(feof(file)) {
      break;
    }
    int i = 0;
    while(routerStruct[connR1] -> Pointers[i] != ptr && i < sizeof(routerStruct[connR1] -> Pointers)) { //bare en teller. Kunne sikkert bruke en annen måte, men orker ikke endre på det fordi det funker
      i++;
    }
    routerStruct[connR1] -> Pointers[i] = connR2; //legger det inn i structen
    // printf("h%d\n", sizeof(routerStruct[connR1] -> Pointers[i]));
    routerStruct[connR1] -> counter = routerStruct[connR1] -> counter + 1; //oppdaterer telleren

    // ptrStruct = &routerStruct[connR2]; //dette brukte jeg får å populere Pointers med pekere til structene, men jeg fikk ikke til å bruke dem i printRouterID så bruker int for RouterID isteden
    // routerStruct[connR1].Pointers[i] = ptrStruct;

  }


  // for (k = 0; k < numb; k++) {
  //   for (l = 0; l < 10; l++) {
  //     if (routerStruct[k] -> Pointers[l] != ptr) {
  //       printf("%d h-> %d\n", k, routerStruct[k] -> Pointers[l]);
  //     }
  //   }
  // }
  return numb; //returnerer antall rutere her så jeg enkelt kan bruke den i andre metoder
}
void printRouterID(int router) { //burde være enkel å forstå
  printf("RouterID: %d\n", routerStruct[router] -> RouterID);
  printf("Model: %s\n", routerStruct[router] -> Model);
  printf("Flag: %d\n", routerStruct[router] -> Flag);
  int i = 0;
  while(i < routerStruct[router] -> counter) {
    printf("Nabo %d: %d\n", i, routerStruct[router] -> Pointers[i]);
    i++;
  }
  printf("\n");
}
void setModel(int router, char *name) {
  strcpy(routerStruct[router] -> Model, name); //endrer navnet i structen så man enkelt kan skrive det ut til filen senere
}
void addConnection(int value1, int value2) {
  routerStruct[value1] -> Pointers[routerStruct[value1] -> counter] = value2; //legger til koblingen  og oppdaterer telleren
  routerStruct[value1] -> counter = routerStruct[value1] -> counter + 1;
  // printf("%d\n", routerStruct[value1] -> Pointers[routerStruct[value1] -> counter]);

}
void delRouter(int router, int numb) {
  int i;
  int j;
  for (i = 0; i < numb; i++) {
    if (routerStruct[i] == NULL) { //hopper over hvis en annen ruter som ligger før allerede er slettet
      i++;
    }
    for (j = 0; j < 10; j++) {
      if (routerStruct[i] -> Pointers[j] == router) {
        routerStruct[i] -> Pointers[j] = i; //enkelt å flitrere bort de som peker på seg selv siden det er en funksjonalitet som aldri brukes
      }
    }
  }
  free(routerStruct[router]);
  routerStruct[router] = NULL; //enkel måte å slette ruteren på, men fant ikke en måte å slette koblingene på uten å lage kaos
}

void write(FILE *file, int numb) {
  fwrite(&numb, 4, 1, file);
  unsigned int i;
  for (i = 0; i < numb; i++) {
    if (routerStruct[i] == NULL) { //hvis man sletter en ruter så går man bare videre til den neste
      i++;
    }
    unsigned int routerID = routerStruct[i] -> RouterID;
    unsigned char flag = routerStruct[i] -> Flag;
    unsigned char length = strlen(routerStruct[i] -> Model);
    char model[length];
    strcpy(model, routerStruct[i] -> Model);
    //denne er veldig lik read-metoden så kommenterer ikke funksjonaliteten her

    fwrite(&routerID, 4, 1, file);
    fwrite(&flag, 1, 1, file);
    fwrite(&length, 1, 1, file);
    fwrite(&model, length, 1, file);
    fwrite("\0", 1, 1, file);

  }
  for (i = 0; i < numb; i++) {
    unsigned int j;
    for (j = 0; j < 10; j++) {
      if (routerStruct[i] != NULL && routerStruct[i] -> Pointers[j] != routerStruct[i] -> Pointers[routerStruct[i] -> counter + 1] && routerStruct[i] -> Pointers[j] != i) { //denne må sjekke om ruteren er fjernet og om det er en nullpeker og om det er noen som peker på seg selv som da betyr at den er fjernet
        fwrite(&i, 4, 1, file); //når jeg skriver til fil her så blir den helt lik, men hvis jeg bruker diff så er det noen bytes som blir annerledes. Jeg sjekket ved å lese inn den filen jeg skrev ut til og fikk da samme resultat hver gang. Jeg skjønner ikke hvorfor det ikke blir likt, men det funker å lese det inn.
        unsigned int r2 = routerStruct[i] -> Pointers[j];
        fwrite(&r2, 4, 1, file);
        fwrite("\0", 1, 1, file);

        // printf("%d\n", sizeof(routerStruct[i] -> Pointers[j]));
        // printf("%d -> %d\n", i, r2);
      }
    }
  }
}

void setFlag(int router, int pos, int change) {
  int flag = routerStruct[router] -> Flag;
  if (change == 0) {
    flag = (254 << pos) & flag; //bruker en AND funksjon for å bytte tallet som er der til 0
  }
  else {
    flag = (1 << pos) | flag; //bruker OR her for å bytte til 1
  }

  routerStruct[router] -> Flag = flag; //legger det inn i strukten
  // printf("%d\n", flag);
}


void DFS(int i) {
  int j;
  // printf("%d\n", routerStruct[i] -> visited);
  routerStruct[i] -> visited = 1;
  for (j = 0; j < routerStruct[i] -> counter; j++) {
    if (routerStruct[routerStruct[i] -> Pointers[j]] != NULL && routerStruct[routerStruct[i] -> Pointers[j]] -> visited == 0) {
       DFS(routerStruct[i] -> Pointers[j]);
}
  }
}
void isRoute(int router1, int router2, int numb) {
  int i;
  // printf("%d\n", routerStruct[1] -> Pointers[1]);
  // printf("%d\n", routerStruct[1] -> counter);
  for (i = 0; i < numb; i++) {
    if (routerStruct[i] != NULL) {
      routerStruct[i] -> visited = 0;
    }
    i++;
  }
  if (routerStruct[router1] != NULL && routerStruct[router2] != NULL) {
    DFS(router1);
    if (routerStruct[router2] -> visited != 1) {
      printf("Det finnes ingen rute mellom %d og %d\n", router1, router2);
    }
    else {
      printf("Det finnes en rute mellom %d og %d\n", router1, router2);
    }
  }
  else {
    printf("finner ikke ruterene\n");
  }
}

int readCommands(FILE *fileCommands, FILE *routerFile, int numb) {
  char command[255];
  char commandType[255];
  char *ptr;
  int value1;
  int value2;
  int value3;
  int newNumb = numb;
  char delim[] = " ";
  while(1) {
    fgets(command, 255, fileCommands);
    if (feof(fileCommands)) {
      break;
    }
    strcpy(commandType, command);
    ptr = strtok(commandType, delim);
    //printf("%s", command);
    //printf("Type: %s\n", commandType);

    if(strcmp(ptr, "print") == 0) {
      ptr = strtok(command, "print"); //henter ut verdien som ikke er print
      printRouterID(atoi(ptr));
    }

    else if(strcmp(ptr, "sett_modell") == 0) {
      char *name = malloc(248);
      sscanf(command, "%s %d %[^\n]%s", &value1, &value2, name); //henter ut verdiene som og setter dem i verdier.
      // printf("%s %d %s\n", &value1, value2, name);
      setModel(value2, name);
      free(name);

    }
    else if(strcmp(ptr, "sett_flag") == 0) {
      ptr = strtok(NULL, "sett_flag");
      sscanf(ptr, "%d %d %d", &value1, &value2, &value3);
      setFlag(value1, value2, value3);
      // printf("sett flagg: %d %d %d\n", value1, value2, value3);

    }
    else if(strcmp(ptr, "finnes_rute") == 0) {
      ptr = strtok(NULL, "finnes_rute");
      sscanf(ptr, "%d %d %d", &value1, &value2);
      isRoute(value1, value2, numb);
      // printf("rute: %d %d\n", value1, value2);

    }
    else if(strcmp(ptr, "legg_til_kobling") == 0) {
      ptr = strtok(NULL, "legg_til_kobling");
      sscanf(ptr, "%d %d %d", &value1, &value2);
      // printf("kobling: %d %d\n", value1, value2);
      addConnection(value1, value2);

    }
    else if(strcmp(ptr, "slett_router") == 0) {
      ptr = strtok(NULL, "slett_router");
      sscanf(ptr, "%d", &value1);
      delRouter(value1, numb);
      newNumb--;
    }
  }
  return newNumb;
}


int main(int argc, char** argv) {
  FILE *routerFile;
  FILE *commandFile;
  char *fileName1 = argv[1];
  char *fileName2 = argv[2];


  routerFile = fopen(fileName1, "rb");
  commandFile = fopen(fileName2, "r");
  if(routerFile == NULL) {
    printf("Could not open first file\n");
    exit(1);
  }
  if(commandFile == NULL) {
    printf("Could not open second file\n");
    exit(1);
  }

  int numb = read(routerFile);
  int newNumb = readCommands(commandFile, routerFile, numb);
  fclose(routerFile);
  routerFile = fopen(fileName1, "wb");
  // FILE *routerFile1 = fopen("Test", "wb");
  // printf("%d\n", newNumb);
  write(routerFile, newNumb);
  int i;
  while (i <= numb) { //free-er alle structene som jeg mallocet tidligere i read-funksjonen
    free(routerStruct[i]);
    i++;
  }
  // fclose(routerFile1); //lukker alle filene
  fclose(routerFile);
  fclose(commandFile);
}
