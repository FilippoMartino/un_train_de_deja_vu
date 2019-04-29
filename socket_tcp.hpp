#include "address.hpp"

#include <unistd.h>
#include <list>
#include <iterator>

using namespace std;

#ifndef __SOCKETTCP_HPP
#define SOCKETTCP_HPP

#define MAX_CONNECTIONS 50
#define DEFAULT_SERVER_PORT 8080
#define MAX_BUFFER_SIZE 4096

//TODO Implementare i distruttori


class SocketTcp{

    protected:  int sock_id;

    protected:  SocketTcp();
                ~SocketTcp();

                void setBroadcast(bool);

};

SocketTcp::SocketTcp(){

   /*
        Costruttore della classe principale, imposta il socket:
        - Famiglia: AF_INET
        - Tipo di trasmissione: SOCK_STREAM
        - Flag: 0

        Gestisce mediante errno eventuali errori d'esecuzione

   */

   this->sock_id = socket(AF_INET, SOCK_STREAM, 0);

   if (sock_id == -1){
       printf("Error setting socket: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
   }

}

// Distruttore
SocketTcp::~SocketTcp(){

  //mi limito a chiudere il socket
  close(this->sock_id);

}

void SocketTcp::setBroadcast(bool is_active){


        /*
            Controlla il valore passato alla funzione,
            se l'utente desidera settare il broadcast sul rispettivo
            socket allor si utilizza la funzione setsockopt():

            - Identificativo del socket sul quale impostare il broadcast: this->sock_id
            - Macro per manipolare il socket a livello di API: SOL_SOCKET
            - La caratteristica che si desidera modificare: SO_BROADCAST
            - Il valore per il quale cambiare la suddetta opzione (referenza): &isActive
            - Grandezza del tipo di valore che abbiamo passato: sizeof(bool)

            Naturalmente l'esecuzione della funzione viene, in caso di errori, gestita,
            segnalando all'utente il problema incorso e terminando l'applicazione [se si desidera
            cambiare il valore di ritorno del metodo e non terminare il programma ricordarsi di
            cambiare il tipo restituito dal metodo]

        */

		if (is_active){

			if(setsockopt(
                this->sock_id,
                SOL_SOCKET,
                SO_BROADCAST,
                &is_active,
                sizeof(bool)) == -1 ){

                    printf("Error setting broadcast: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);

                }

		}

	}

  //============================================Connection===================================================================

  class Connection{

      protected:

              int conn_id;

      public:
              Connection(int);
              ~Connection();

              bool send_message(char*);
              //indicare il nome del file
              bool send_html(char*);
              bool send_raw(void*,long int);
              bool send_file(char*);

              char* receive_message();
              char* receive_raw(int*);
              //salva il file con il nome specificato
              bool receive_file(char* file_name);

      private:
              //calcola la grandezza di un file passandogli il path come parametro
              long int calculate_file_size(char* file_path);


  };

  Connection::Connection(int id){

    this->conn_id = id;

  }

  Connection::~Connection(){}

  bool Connection::send_raw(void* buffer, long int buffer_size){

    //DEBUG
      //printf("File da spedire: %s\n", (char*) buffer);

      if(send(this->conn_id,
              buffer,
              buffer_size,
              0) == -1){

        printf("Error sending buffer: %s\n", strerror(errno));
        return true;
      }


      return false;
  }

  bool Connection::send_message(char* message){

    return send_raw(message, (long int) strlen(message) + 1);

  }

  bool Connection::send_html(char* path){

    FILE* file = fopen(path,"rb");
    long int file_size =  calculate_file_size(path);
    //buffer in cui verrà salvato l'intero file
		char buffer[file_size];
		//leggo tutto il file e lo metto nel buffer
		fread(buffer, file_size, sizeof(char), file);
    fclose(file);
    send_message((char*) buffer);

  }

  //Codice bovinamente riciclato dalla socket_udp
  bool Connection::send_file(char* path){

		//apriamo il file passato dall'utente in modalità lettura
		//molto importante il rd, read binary
		FILE* file = fopen(path,"rb");

		//calcolo la dimensione del file mediante la funzione 'calculate_file_size'
		long int file_size =  calculate_file_size(path);

    //invio la dimensione del file che sta per arrivare (intercetto eventuale errore)
    if (send_raw((char*) file_size, (sizeof(long int))))
      printf("Error sending file dimension: %s\n", strerror(errno));


		//buffer in cui verrà salvato l'intero file
		char buffer[file_size];

		//leggo tutto il file e lo metto nel buffer
		fread(buffer, file_size, sizeof(char), file);

    //DEBUG
    //printf("[socketTcp: ho letto: %s]\n", buffer);

    //chiudo il file
		fclose(file);

    return send_raw((char*) buffer, file_size);

  }


  char* Connection::receive_raw(int* buffer_size){

    char buffer[MAX_BUFFER_SIZE + 1];

    //Nel valore puntato da buffer_size inserisco il numero di bit ricevuti
   *buffer_size = recv(this->conn_id,
                       (void*) buffer,
                       MAX_BUFFER_SIZE,
                       0);

    if(*buffer_size == -1)
      printf("Error receiving buffer: %s", strerror(errno));

    char* ret = (char*) malloc(*buffer_size + 1);

    for(int i=0; i <= *buffer_size; i++){

        *(ret + i) = buffer[i];

    }

    return ret;


  }

  bool Connection::receive_file(char* file_name){

		//riceviamo innanzitutto la grandezza del file
    long int  file_size;
    int long_int_size = sizeof(file_size);
		file_size = (long int) receive_raw(&long_int_size);

		//creo il buffer che conterrà l'intera immagine
		char buffer[file_size];

    /*  Sarà compito di tcp eventualmente spacchettare il file in arrivo
        ma al nostro programma non da prblemi per via dell'indipendenza
        dei layer.
        Concateniamo il file in arrivo direttamente nel buffer (le dimensioni
        del buffer già perchè ricevute in precedenza) perchè:

        buffer[0] <-- char* receive_raw()

    */

    strcat(buffer, receive_raw( (int*) &file_size));

		//apro il file con il nome passatoci dal mittente
		FILE* my_file = fopen(file_name, "w");

		//trascrivo l'intero buffer sul nuovo file
		fwrite(buffer, sizeof(buffer), sizeof(char), my_file);
		//chiudo il file
		fclose(my_file);

		return false;

}

  char* Connection::receive_message(){

    int message_lenght;
    char* message = receive_raw(&message_lenght);
    //Ulteriore controllo sul carattere terminatore
    //Sposto il puntatore sull'ultima cella e vado ad aggiungerlo
    *(message + message_lenght) = '\0';
    //restistuisco il messaggio
    return message;

  }

  long int Connection::calculate_file_size(char* file_path){

    //apro il file in modalità lettura
    FILE* file = fopen(file_path, "r");

    //controlliamo che il file passato dall'utente esista effettivamente
    if (file == NULL) {
      printf("File Not Found!\n");
      return -1;
    }

      /*
      per utilizzo di questa fuzione vedre es calcolare_dimesione_file
      nella cartella esercizi
      */

      fseek(file, 0L, SEEK_END);

      // calcoliamo la grandezza del file passato mediante ftell()
      long int file_size = ftell(file);

      // chiudo il file
      fclose(file);

      return file_size;

  }

  //======================================ClientConnection===================================================================

  class ClientConnection:public Connection{

  public: ClientConnection(int);
          ~ClientConnection();

  };

  ClientConnection::ClientConnection(int conn_id):
    Connection(conn_id){}

  ClientConnection::~ClientConnection(){}

  //======================================ServerConnection===================================================================

  class ServerConnection:public Connection{

    public: ServerConnection(int);
  		      ~ServerConnection();
  };

  ServerConnection::ServerConnection(int conn_id):Connection(conn_id){}

  ServerConnection::~ServerConnection(){

  	shutdown(conn_id, SHUT_RDWR);

  }


//==============================================ServerTcp==================================================================

class ServerTcp: public SocketTcp{

    private:  list <ServerConnection*> connections;
              char* my_ip;

    public:   ServerTcp(int);
              ~ServerTcp();
              ServerConnection* accept_connection();
              //Chiude una singola connessione
              void disconnect(ServerConnection*);
              //Chiude tutte le connessioni presenti nel server
              void server_shutdown();
              //TODO da implementare le due funzioni sottostanti
              //bool send_multicast_message(char*);
              //bool send_multicast_raw(void*, int);

    private:  void close_connections();

};

//associo il socket all'indirizzo
ServerTcp::ServerTcp(int port){

    //Mi costruisco un indirizzo con la classe Address
    Address my_address(strdup(IP_LO), port);
    //Inserisco il risultato nella struttura binaria
    struct sockaddr_in my_self = my_address.getBinary();
    //associo indirizzo a socket
    if (bind(sock_id,
            (struct sockaddr*) &my_self,
            (socklen_t) sizeof(struct sockaddr_in))
            == -1)
    //Intercetto eventuale presenza errore
    printf("Error doing bind(): %s\n", strerror(errno));

    //Metto il server in ascolto
    if (listen(sock_id, MAX_CONNECTIONS) == -1)

      //Intercetto eventuale presenza errore
      printf("Error listening for client: %s\n", strerror(errno));


}

//Chiude tutte le connessioni legate a questo socket
ServerTcp::~ServerTcp(){

  server_shutdown();

}

ServerConnection* ServerTcp::accept_connection(){

  //Struttura in cui finisce l'indirizzo del client che ci sta contattando
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(struct sockaddr);

  //chiamo la API ed intercetto eventuali errori
  int conn_id = accept(sock_id,
                      (struct sockaddr*) &client_address,
                      &client_address_size);

  if (conn_id == -1)
    printf("Error accepting connection: %s\n", strerror(errno));

  //Creo la nuova connessione con il conn_id accettato
  ServerConnection* ret = new ServerConnection(conn_id);
  //Aggiungo la connessione alla lista
  connections.push_front(ret);

  //Restituisco la nuova connessione
  return ret;

}

void ServerTcp::disconnect(ServerConnection* to_disconnect){

  delete(to_disconnect);
	connections.remove(to_disconnect);

}

void ServerTcp::server_shutdown(){

  //Cicla finchè la lista delle connessioni non è vuota
  while (!connections.empty()){
    //disconnette la connessione all'inizio della lista
    disconnect(connections.front());
  }

}


//==============================================ClientTcp===================================================================

class ClientTcp:private SocketTcp{

    //Puntatore ad una connessione client, inizializzato a null
    private: ClientConnection* connection = NULL;

    protected:  ClientTcp();
                ~ClientTcp();

                /*
                * Questi metodi sono in realtà fittizzi, per ognuno
                * ci si appoggia alla classe che gestisce le connessioni
                * del client, a cui si accede mediante la maniglia 'connection',
                * che viene inizializzata quando di invoca il metodo connect
                *
                */

                bool connect_to_server(Address);
                bool send_message(char*);
                bool send_raw(void*, int);
                char* receive_message();
                char* receive_raw(int*);
};

ClientTcp::ClientTcp():SocketTcp(){}

ClientTcp::~ClientTcp(){

  if(connection != NULL)
      delete connection;

}

bool ClientTcp::connect_to_server(Address server_address){

  struct sockaddr_in server = server_address.getBinary();

  //Connetto il socket al server, intercetto eventuali errori
  if (connect(sock_id,
             (struct sockaddr*) &server_address,
             sizeof(struct sockaddr_in)) == -1){

               printf("Error connecting to server: %s\n", strerror(errno));
               return true;

             }

  this->connection = new ClientConnection(sock_id);
  return false;

}

#endif
