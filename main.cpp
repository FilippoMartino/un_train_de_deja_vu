#include "socket_tcp.hpp"
#include <sqlite3.h>

#define MAX_SQL 100
#define MAX_N_THREAD 10
/*Costanti per il check del biglietto*/
#define CODE_OK 0
#define CODE_NOT_EXIST 1
#define CODE_ALREDY_USED 2

#define DB_NAME (char*) "VESPAM"
#define OK_MESSAGE (char*) "OK | Database updated"
#define KO_MESSAGE (char*) "KO | No code"
#define CODE_ALREDY_USED_MESSAGE (char*) "KO | Code already used"

#define CONTROL_PU_IF_EXIST (char*) "SELECT codicePU FROM TICKETS WHERE codicePU = "
#define UPDATE_TABLE (char*) "UPDATE TICKETS SET validation = 1 WHERE codicePU = "

typedef struct{

  ServerConnection* my_server_connection;
  ServerTcp* my_server;

}Params;

/*Uso questa variabile globale per il controllo del codce, in base al
  suo valore del quale la modifico nella funzione di callback*/
int check;

void* thread_callback(void*);

static int callback(void *, int , char **, char **);

int main(int argc, char  *argv[]) {

  ServerTcp* server = new ServerTcp(atoi(argv[1]));
  pthread_t pthread_array[MAX_N_THREAD];
  int ret, i = 0;

  while(1){

    /*Alloco lo spazio in memoria per la struttura*/
    Params* params = (Params*) malloc(sizeof(Params));

    /*Popolo la struttura*/
    params->my_server_connection = server->accept_connection();
    params->my_server = server;
    pthread_t tid;

    int ret = pthread_create(&pthread_array[i], NULL, thread_callback,(void*) params);
    i++;

    for (int j = 0; j <  i; j++ )
      ret = pthread_join(pthread_array[j], NULL);


  }
  /*pthread_join?*/
  /*Non dealloca mai la struttura?*/

  /*Chiudiamo ogni sorta di connessione e liberiamo il server*/
  delete(server);

}

void* thread_callback(void* params){
  /*Cloniamo la variabile comune a tutti i thread params*/
  Params* callback_prams = (Params*) params;
  /*Preleviamo i dati dalla struttura*/
  ServerConnection* callback_connection = callback_prams->my_server_connection;
  ServerTcp* callback_server = callback_prams->my_server;
  /*Riceviamo il codice da controllare dal server*/
  char* codice_pu = callback_connection->receive_message();
  /*Dichiariamo un db e tentiamo di aprirlo gestendone eventuali errori*/
  sqlite3* db;
  if(sqlite3_open(strdup(DB_NAME), &db) != SQLITE_OK){
    printf("Error opening database: %s", sqlite3_errmsg(db));
    exit(EXIT_FAILURE);
  }

  /*Prepariamo la query*/
  char query[MAX_SQL + strlen(CONTROL_PU_IF_EXIST)];
  /*Pezzo senza il codice*/
  strcat(query, CONTROL_PU_IF_EXIST);
  /*Codice*/
  strcat(query, codice_pu);
  /*Aggiunta del ; finale altrimenti sqlite strigna*/
  strcat(query, ";");
  char* error_message;
  sqlite3_exec(db, query, callback, NULL, &error_message);
  if(check == CODE_NOT_EXIST){
    callback_connection->send_message(KO_MESSAGE);
    printf("Database not updated, code %s not exist\n", codice_pu);
  }else if(check == CODE_ALREDY_USED){
    /*significa che il codice è già stato utilizzato*/
    callback_connection->send_message(CODE_ALREDY_USED_MESSAGE);
    printf("Database not updated, code %s is been already used\n", codice_pu);
  }else{
      /*In caso POSITIVO*/
      char update_query[MAX_SQL + strlen(UPDATE_TABLE)];
      /*Query di aggiornamento meno il codice pu*/
      strcat(update_query, UPDATE_TABLE);
      /*Aggiunta del codice pu*/
      strcat(update_query, codice_pu);
      /*Aggiunta del ; per il motivo di cui sopra*/
      strcat(update_query, ";");
      /*Controlliamo che la query sia avvenuta senza problemi*/
      if(sqlite3_exec(db, query, callback, NULL, &error_message) != SQLITE_OK)
        printf("Error updating database: %s\n", error_message);
      else
        printf("Database updated successfully, pu_code used: %s\n", codice_pu);
      /*Deallochiamo la variabile errore*/
      /*Avvisiamo il client dell'avvenuta modifica del db e diamo l'OK*/
      callback_connection->send_message(OK_MESSAGE);
  }
  /*Chiudiamo il db*/
  sqlite3_close(db);
  /*Chiudiamo la connessione*/
  callback_server->disconnect(callback_connection);
  /*Deallochiamo la struttura duplicata*/
  free(callback_prams);
  pthread_exit(NULL);
}

/*Dato che facciamo solo l'update non la usiamo, perciò è vuota*/
static int callback(void *data, int argc, char **argv, char **azColName){

    /*Se non ci sono risultati*/
    if (argc == 0)
      check = CODE_NOT_EXIST;

    /*DEBUG*/
    // printf("argc: %d\n", argc);
    // for(int i = 0; i < argc; i++)
    //   printf("Col_name = %s, value = %s\n", azColName[i], argv[i]);

    else if (atoi(argv[0]) == 1)
      check = CODE_ALREDY_USED;

    else
      check = CODE_OK;

    return 0;
}
