# un_train_de_deja_vu
Codice per verifica TPSI

Simulazione server che accetta richieste da parte di più client che inviano un codice:
il server deve accedere ad un database con le API sqlit3, controllare se il codice esiste e se non 
è ancora stato utilizzato (il controllo avviene su un campo del db: validation, che può essere 0 o 1), inoltre se non è ancora stato utilizzato deve aggiornare il campo validation e successivamente mandare al client una risposta:

se il codice non esiste: KO | No code

se il codice esiste ma è già stato utilizzato: KO | Code already used

se invece il codice esiste e non è ancora stato usato: OK | Database updated
