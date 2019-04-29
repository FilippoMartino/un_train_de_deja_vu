
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <errno.h>

        /*
        *   <> -> parentesi acute
        *   Per salvare le librerie c++ utilizziamo
        *   l'estenzione .hpp
        *   Le librerie uscite prima del 2011 invece
        *   non hanno un'estensione
        *
        */

    //l'underscore doppio si dice dunder
    //i costruttori si dicono dunderNOMECLASSEunderscore

    #ifndef __ADDRESS_HPP
    #define __ADDRESS_HPP

    static pthread_mutex_t mutex;


    /*
    *
    *   IP_LO -> indirizzo di loopback
    *
    *           se spediamo a questo inidirizzo sarà
    *           sempre la stessa macchina a ricevere
    *           il messaggio, perchè non ecse dalla rete
    *
    *           è molto utile per effettuare delle prove
    *           sulla macchina
    *
    *   IP_DHCP -> indirizzo che indica una richiesta di tipo
    *              broadcast che provoca una richiesta da parte
    *              di un mittente di un nuovo indirizzo ip
    *
    */

    #define IP_LO "127.0.0.1"
    #define IP_DHCP "0.0.0.0"

    #define PORT_ZERO 0


    //TODO CREARE UN MAIN CHE TESTI LA LIBRERIA TDD [TEST DRIVEN DEVELOPEMENT]


    class Address {

        /*
        *
        * gli attributi non necessitano inizializzazioni
        *
        */

        private:    char* ip;
                    int port;
        /*
        *
        *   Si dichiarano tutti i prototipi e successivamente
        *   tutti i metodi, non necessita dei nomi delle variabili
        *
        */

        public:     Address(char*, int); //metodo per nuova istanza
                    Address(); //metodo vuoto di defalut

                    /*
                    *
                    *   Simile a quella per referenza, la
                    *   seguente è una chiamata per RIFERIMENTO
                    *   la & serve per permettere la chiamata dai
                    *   metodi con il '.' altrimenti dovremmo usare
                    *   il '->'
                    *
                    */

                    Address(const Address&);

                    /*
                    *
                    *   Implementiamo anche un costruttore che riceva una
                    *   struttura di tipo sockaddr_in che richiami il metodo
                    *   di set, estrapolando così i dati che ci servono
                    *
                    */

                    Address(struct sockaddr_in);


                    char*   getIp();
                    int     getPort();
                    void    setIp(char*);
                    void    setPort(int);

                    char*   toString();

                    void    setBinary(struct sockaddr_in);
                    sockaddr_in getBinary();

                     void inet_ntoa_mutex(struct sockaddr_in socket);

                    //tilde sotto windows alt + 126
                    //tilde sotto unix altGr + ì

                    ~Address();








    }; //mettere un ; alla fine della dichiarazione

                /*
                *
                *   Adesso dobbiamo implementare tutti i metodi
                *   Dobbiamo specificare al compilatore che si trova
                *   davanti ad un metodo, quindi dobbiamo specificare
                *   (anche nel costruttore), il tipo di classe a cui appartiene
                *   se mancasse la specifica il compilatore penserebbe di
                *   essere davanti ad una funzione
                *
                */



                Address::Address(char* ip, int port) {

                    /*
                    *
                    *   L'operatore 'this' è una maniglia
                    *   all'oggetto stesso, che quindi è una
                    *   referenza a se stesso, dobbiamo quindi
                    *   riferirci ad esse con il '->'
                    *
                    */

                    this->ip = strdup(ip);
                    this->port = port;

                }

                Address::Address(){

                    this->ip = strdup(IP_LO);
                    this->port = PORT_ZERO;

                }

                Address::Address(const Address &a){

                /*
                *
                *   Tendenzialmente non si usano i set e get nel costruttore.
                *   All'interno del costruttore per copia le proprietà ricevute
                *   per parametro sono accessibili
                *
                */

                    this->ip = strdup(a.ip);
                    this->port = a.port;

                }

                Address::Address(struct sockaddr_in socket){

                    setBinary(socket);

                }


                /*
                *
                *   Non si può richiamare il distruttore se non si è fatta
                *   una new.
                *   Importante!
                *   non è necessaria la free sulla porta perchè è un tipo di dato primitivo
                *
                */

                Address::~Address(){

                    free(this->ip);

                }
                /*
                *
                *   Da qui in poi partono tutti i metodi accessors che
                *   implementano la possibilità di lavorare con l'oggetto
                *   address.
                *   Quando in c++ vogliamo scrivere dei metodi, dobbiamo innanzitutto
                *   dichiararne i prototipi all'interno della classe principale dell'oggetto
                *   perchè il compilatore c++, come quello del c, fa 'una sola passata' e
                *   quindi legge immediatamente i prototipi
                *
                *   Quando andiamo a dichiarare la struttura vera e propria del medodo
                *   dobbiamo innanzitutto specificarne il tipo, come prima cosa.
                *   Successivamente andiamo ad indicare il nome della classe con i '::'
                *   per avvisare il compilatore che quello che sta per leggere è un metodo
                *   e non una semplice funzione.
                *   Una volta fatto il passaggio sopra andiamo a mettere il nome del metodo
                *   già specificato nella classe, e gli eventuali parametri di cui necessita
                *   per il funzionamento; questi ulitmi possono non avere un nome di identificazione
                *   nella dichiarazione del prototipo, ma sono obbligati ad avere un nome per poter essere
                *   utilizzati d'ora in poi, esempio:
                *   [void NomeClasse::setQualcosa(char* QUI_DEVO_SPECIFICARE_IL_NOME)]
                *
                */

                char* Address::getIp(){

                    return strdup(ip);

                }

                int Address::getPort(){

                    return port;

                }

                void Address::setIp(char* ip){

                    this->ip = strdup(ip);


                }

                void Address::setPort(int port){

                    this->port = port;

                }

                char* Address::toString(){

                    char return_message[4096];

                    sprintf(return_message, "%d - %s", this->port, this->ip);

                    return strdup(return_message);


                }

                /*
                *
                *   Aggiungiamo adesso i due metodi che permettano di
                *   lavorare con le strutture soket_ddr()
                *
                */

                void Address::setBinary(struct sockaddr_in socket){

                    this->port = ntohs(socket.sin_port);

                    /*

                        Il problema della inet_ntoa è che non è thread-safe
                        Restituisce l'indirizzo della stringa statica che corrisponde
                        al valore sin_addr in char* (è sempre lo stesso STATICO, per
                        questo necessita della strdup();)

			Si appoggia inoltre alla funzione inet_ntoa_mutex che estrapola
			l'inidirizzo dalla struttura rendendolo thread safe

                    */

                    pthread_mutex_init(&mutex, NULL);

                    inet_ntoa_mutex(socket);

                    pthread_mutex_destroy(&mutex);

                }

                sockaddr_in Address::getBinary(){

                    sockaddr_in socket;

                    socket.sin_family = AF_INET;

                    //inseriamo l'indirizzo passandolo nel soket passando lo stesso per referenza
                    inet_aton(strdup(this->ip), &socket.sin_addr);
                    //convertiamo il valore della porta e lo inseriamo
                    socket.sin_port = htons(this->port);

                    //rempiamo il filler con degli 0
                    for(int i = 0; i < 8; i++)
                        socket.sin_zero[i] = 0;

                    return socket;

                }

		/*
			questa funzione serve semplicemente a ricavare
			l'indirizzo dalla struttura sockaddr_in in modo thread
			safe
		*/

                void Address::inet_ntoa_mutex(struct sockaddr_in socket){

                    pthread_mutex_lock(&mutex);

                    this->ip = strdup(inet_ntoa(socket.sin_addr));

                    pthread_mutex_unlock(&mutex);

                }



    #endif // __ADRESS_HPP
