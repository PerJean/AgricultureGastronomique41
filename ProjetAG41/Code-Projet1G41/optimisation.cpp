#include "optimisation.h"
#include <fstream>
#include <vector>
#include "Random.h"
#include "site.h"
#include "SolFrequence.h"

using namespace std;

///m�thode de descente simple pour l'allocation des fr�quences

//cette methode explore ls site 1 a la suite de l'autre selon leur ordre de stockage de fichier
//de donn�es (DataFT/ListeStations_V1_900Mhz), on �num�re toutes les combinaisons de fr�quences
//possible sur chaque site, on choisit la meilleure configuration en fr�quence, on retient cette configuration
//et en passe au site suivant, jusqu'au 36eme site. une fois tous les sites sont parcourru, on passe � l'it�ration suivante
//l'algorthime s'arr�te des qu'il n'y a pas d'am�lioration � la derniere it�ration

void optimisation::descente_simple_frequence(char* nom, int stable,
											 pointTest** lesPTA, int nb_tp_a,
											 secteur** lesSecteurA, int nb_secteur_a, int no_scen) {
	GOutputFile file_sortie(nom);
	file_sortie.open();
    file_sortie << "Optimisation robuste des fr�quences" << "\n";
    file_sortie << "les parametres de l optimisation" << "\n";
    file_sortie <<"12h15"<< "\n";
    file_sortie << "facteur de stabilite= " << stable << "\n";
    file_sortie << "le critere est: le nombre de clients non couverts"<< "\n";
	double nb_clients_non_couvert = 0.0;
	double best_nb_clients_non_couvert = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen);
	double best_nb_clients_non_couvert2 = best_nb_clients_non_couvert+1;
    int nbIteration = 0;
	while (best_nb_clients_non_couvert != best_nb_clients_non_couvert2) {
		best_nb_clients_non_couvert2 = best_nb_clients_non_couvert;
		file_sortie<< "le nombre d iteration " << nbIteration++ << "\n";
		for (int i=0; i<site::size; i++) {
			file_sortie << "site " << i << " nb sect = " << site::lesSites[i]->get_nb_secteur() << "\n";
			if (site::lesSites[i]->get_nb_secteur() == 1) {
				int old_p = site::lesSites[i]->get_secteur(0)->get_porteuse();
				int best_p = old_p;
	//			best_nb_clients_non_couvert = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a);
				for (int p=0; p<Environement::nbProteuses; p++) {
					if (p != old_p) {
					    site::lesSites[i]->get_secteur(0)->set_porteuse(p);
//						nb_clients_non_couvert = Fitness::eval(stable,lesSecteurA, nb_secteur_a, site::lesSites[i]->get_secteur(0));
                        nb_clients_non_couvert = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen);
						if (best_nb_clients_non_couvert>nb_clients_non_couvert) {
						    file_sortie << "le site" << i<< "\n";
							file_sortie << (float)best_nb_clients_non_couvert << "Nbre clients non couverts -> " << (float)nb_clients_non_couvert << "\n";
							file_sortie<< "non couverts t = " << (float)Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a) << "\n";
							best_nb_clients_non_couvert=nb_clients_non_couvert;
							best_p = p;
						}
					}
				}
				if (best_p != Environement::nbProteuses-1)
					site::lesSites[i]->get_secteur(0)->set_porteuse(best_p);
			}
			else {
				int k = 0, l = 1;
				int permutation = 3;
				int nb_permutations = MathB::fact(permutation)-1;
				int *best_p = (int*)malloc(permutation*sizeof(int));
				int *les_p = (int*)malloc(permutation*sizeof(int));
				for (int j=0; j<site::lesSites[i]->get_nb_secteur(); j++)
					les_p[j] = site::lesSites[i]->get_secteur(j)->get_porteuse();
				if (site::lesSites[i]->get_nb_secteur()==2) les_p[2] = 2;
				 file_sortie << "site " << i << " nb sect = " << site::lesSites[i]->get_nb_secteur() << "\n";
				for (int j=0; j<permutation; j++)
					best_p[j] = les_p[j];
				for (int ii=0; ii<nb_permutations; ii++) {
					int tmp = les_p[k];
					les_p[k] = les_p[l];
					les_p[l] = tmp;
					if (site::lesSites[i]->get_nb_secteur()>k) site::lesSites[i]->get_secteur(k)->set_porteuse(les_p[k]);
					if (site::lesSites[i]->get_nb_secteur()>l) site::lesSites[i]->get_secteur(l)->set_porteuse(les_p[l]);
					nb_clients_non_couvert = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen);
					if (best_nb_clients_non_couvert>nb_clients_non_couvert) {
					    file_sortie << "le site" << i<< "\n";
						file_sortie << (float)best_nb_clients_non_couvert << " --> " << (float)nb_clients_non_couvert << "\n";
						file_sortie << "freq=" << les_p[0] << " " << les_p[1] << " " << les_p[2] << "\n";
						file_sortie << "non couverts t = " << (float)Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a) << "\n";
                        file_sortie << "non couverts 1 = " << (float)Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, 0) << "\n";
						best_nb_clients_non_couvert=nb_clients_non_couvert;
						for (int j=0; j<site::lesSites[i]->get_nb_secteur(); j++)
							best_p[j] = les_p[j];
					}
					k = (k+1)%permutation;
					l = (l+1)%permutation;
				}
				for (int j=0; j<site::lesSites[i]->get_nb_secteur(); j++)
					site::lesSites[i]->get_secteur(j)->set_porteuse(best_p[j]);
				free(best_p);
				free(les_p);
			}
		}

	}
	file_sortie.close();

}


/// Impl�mentation de la m�thode recherche tabou pour l'allocation des frequences
// Liste des sites voisins du site choisi
//Le liste tabou contenant le meilleur site voisins et la duree qui lui correspond
struct TabuItem{
    TabuItem():conf(NULL),dureeTabu(0){}
    int* conf;
    int conf_taille;
    int dureeTabu;
};


struct ListeTabuItems{
    ListeTabuItems():ListeItems(NULL),nbItems(0){}
    TabuItem** ListeItems;
    int nbItems;
};

void add_Item_ListeTabuItems(ListeTabuItems* liste,int* conf, int dureeTabu){
    if(liste->nbItems == 0){
        liste->ListeItems = new TabuItem*[1];
        liste->nbItems = 1;
    }
    else{
        TabuItem** tmpListe = new TabuItem*[liste->nbItems + 1];
        for(int i=0; i<liste->nbItems; i++)
            tmpListe[i] = liste->ListeItems[i];
        liste->nbItems++;
        delete liste->ListeItems;
        liste->ListeItems = tmpListe;
    }
    TabuItem* item = new TabuItem();
    item->conf = conf;
    item->dureeTabu = dureeTabu;
    liste->ListeItems[liste->nbItems - 1] = item;
}

void update_Items_ListeTabuItems(ListeTabuItems* liste){
    int new_nbItems=0;
    for(int i=0; i<liste->nbItems;i++){
        if(liste->ListeItems[i]->dureeTabu != 0) new_nbItems ++;
    }
    TabuItem** tmpListe = new TabuItem*[new_nbItems];
    TabuItem* item = NULL;
    int index=0;
    for(int i=0; i<liste->nbItems;i++){
        if(liste->ListeItems[i]->dureeTabu != 0){
            item = new TabuItem();
            item->conf = liste->ListeItems[i]->conf;
            item->dureeTabu = liste->ListeItems[i]->dureeTabu - 1;
            tmpListe[index] = item;
            index++;
        }
    }
    liste->nbItems = new_nbItems;
    delete liste->ListeItems;
    liste->ListeItems = tmpListe;
}

void delete_item(TabuItem* item){
    delete item->conf;
    delete item;
}

void delete_ListeTabuItems(ListeTabuItems* liste){
    for(int i=0; i<liste->nbItems;i++)
        delete_item(liste->ListeItems[i]);
    delete liste;
}


typedef struct Table_sites {
	Table_sites():no_site(-1),dist_site_m(INFINITY){}
	int no_site;
	double dist_site_m;
    };

/// Recherches les nb sites les plus proches dun site donn�
void find_closest_site(int num_site, int nb, Table_sites* &closest_sites){
    int nb_closest_site = nb;
    //Table_sites* closest_sites;
    closest_sites = new Table_sites[nb];
    double _dist=0;//, dist_min = INFINITY;
    for (int i=0; i < site::size; i++) {
        if (((site::lesSites[i]->get_x())!= (site::lesSites[num_site]->get_x()))||((site::lesSites[i]->get_y())!=(site::lesSites[num_site]->get_y()))){
        _dist = MathB::dist(site::lesSites[i]->get_x(), site::lesSites[i]->get_y(), site::lesSites[num_site]->get_x(),  site::lesSites[num_site]->get_y());
        int j=nb;
        while ((j>0) && (_dist < closest_sites[j-1].dist_site_m)) j--;
        if (j<nb) {
            for (int k=nb-1; k>j; k--){
                closest_sites[k].dist_site_m = closest_sites[k-1].dist_site_m;
                closest_sites[k].no_site = closest_sites[k-1].no_site;
            }
            closest_sites[j].dist_site_m = _dist;
            closest_sites[j].no_site = i;
        }
        }
    }
}

void find_secteur_from_site(int no_site,secteur** lesSecteurA, int nb_secteur_a,secteur** &secteurs)
{
    int index_secteur = 0;

    for(int i=0; i<nb_secteur_a;i++)
    {
        if(index_secteur == 3)break;

        if(lesSecteurA[i]->get_site()->get_no() == no_site)
        {
            secteurs[index_secteur] = lesSecteurA[i];
            index_secteur++;
        }
    }
    for(index_secteur; index_secteur<3; index_secteur++)
            secteurs[index_secteur]= NULL;
}

bool test_egal_conf(int* conf1,int* conf2, int nb_secteur_a)
{
    for(int i=0; i<nb_secteur_a;i++)
        if(conf1[i] != conf2[i])
            return false;
    return true;
}

bool test_is_in_tabu(int* &conf,int nb_secteur_a,ListeTabuItems* &listeTabu)
{
    for(int i=0; i<listeTabu->nbItems;i++)
    {
        if(test_egal_conf(conf,listeTabu->ListeItems[i]->conf,nb_secteur_a))
            return true;
    }
    return false;
}

double test_permutation(int stable,pointTest** lesPTA,int nb_tp_a,
                        secteur** &lesSecteurA,int nb_secteur_a,int no_scen,
                        ListeTabuItems* &listeTabu,
                        secteur** &secteurs,int index1,int index2)
{

    //return 99999 : pour dire que c'est une �norme fitness

    if(secteurs[index1] == NULL || secteurs[index2] == NULL)
        return 9999;

    int porteuse;

    porteuse = secteurs[index1]->get_porteuse();
    secteurs[index1]->set_porteuse(secteurs[index2]->get_porteuse());
    secteurs[index2]->set_porteuse(porteuse);

    int* conf = new int[nb_secteur_a];
    for(int i = 0; i < nb_secteur_a; i++){
        conf[i] = lesSecteurA[i]->get_porteuse();
    }
    double result = 9999;
    if(test_is_in_tabu(conf,nb_secteur_a,listeTabu) == false)
       result = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen);
    delete conf;


    porteuse = secteurs[index1]->get_porteuse();
    secteurs[index1]->set_porteuse(secteurs[index2]->get_porteuse());
    secteurs[index2]->set_porteuse(porteuse);

    return result;
}



///Impl�mentez votre m�thode ici
/** nom = nom du fichier de sortie
*   stable = on y touche pas. variable de stabilisation de l'affectation d'une
*   antenne � un point test
*   lesPTA = liste de tous les pts test actifs (actif = au moins un client dans le point test
*   nb_tp_a = nombre de pts test actifs
*   lesSecteurA = lste des secteurs comportant au moins un point test actif
*   nb_secteur_a = nombre de secteurs actifs
*   no_scen = le num�ro du sc�nario on n'y touche pas.
*/
void optimisation::frequencyOptimization(char *nom, int stable,
										 pointTest** lesPTA, int nb_tp_a,
										 secteur** lesSecteurA, int nb_secteur_a, int no_scen){

	//USE main.cpp l. 160
    // UTILE secteur::getporteuse() et secteur::getsite()

    cout<<endl<<endl<<endl<<endl;
    cout<<"--------------------------"<<endl<<endl;
    cout<<"Param�tre de la fonction : "<<endl<<endl;

    cout<<"nom : "<<nom<<endl;
    cout<<"stable : "<<stable<<endl;
    cout<<"nb_tp_a : "<<nb_tp_a<<endl;
    cout<<"nb_secteur_a : "<<nb_secteur_a<<endl;
    cout<<"no_scen : "<<no_scen<<endl;

    GOutputFile file_sortie(nom);
	file_sortie.open();
    file_sortie << "Optimisation robuste des fr�quences" << "\n";
    file_sortie << "les parametres de l'optimisation" << "\n";
    file_sortie <<"12h15"<< "\n";
    file_sortie << "facteur de stabilite= " << stable << "\n";
    file_sortie << "le critere est le nombre de clients non couverts"<< "\n";
	double nb_clients_non_couvert = 0.0;
	double best_nb_clients_non_couvert = Fitness::eval(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen);
	double best_nb_clients_non_couvert2 = best_nb_clients_non_couvert+1;
    cout<<endl<<"--------------------------"<<endl;



    ListeTabuItems* listeTabu = new ListeTabuItems();
    Table_sites* voisin = NULL;
    int NB_CLIENT= 1538;
    int NB_ITERATION = 150;
    int DUREE_TABU = 20;

    int* BEST_CONF = new int[nb_secteur_a];
    for(int i = 0; i < nb_secteur_a; i++){BEST_CONF[i] = lesSecteurA[i]->get_porteuse();}
    int BEST_FITNESS = best_nb_clients_non_couvert;

    int ITERATE_SITE = -1;     // pas de changement
    int ITERATE_PERM = 0;     // pas de permutation
    double ITERATE_FITNESS = 9999;

    int BOF_SITE = -1;
    int BOF_PERM = -1 ;
    double BOF_FITNESS = 9999;


    int tmp=-1;
    int nb_site_a=0;
    for(int i=0; i<nb_secteur_a;i++)
    {
        if(lesSecteurA[i]->get_site()->get_no() != tmp)
        {
            nb_site_a++;
            tmp = lesSecteurA[i]->get_site()->get_no();
        }
    }

    int* sites_visites = new int[nb_secteur_a/3];
    int no_site = -1;
    int* etat_site = new int[3];
    secteur** secteurs = new secteur*[3];
    int porteuse;
    double fitness_tmp;
    int index1,index2;
    int* conf =NULL;
    double init_fitness = 0;

    cout << "Fitness initiale: " << best_nb_clients_non_couvert <<"   "<<best_nb_clients_non_couvert/NB_CLIENT*100<<"%  "<<endl;

    for(int iteration=0; iteration<NB_ITERATION;iteration++ )
    {
        //On nettoie la liste des passages
        for( int site=0; site < nb_secteur_a/3; site++)sites_visites[site] = 0;
        ITERATE_SITE = -1;ITERATE_PERM = -1;
        BOF_SITE = -1;BOF_PERM = -1;BOF_FITNESS = 9999;


        //Pour chaque secteur
        int no_secteur=0;
        init_fitness = ITERATE_FITNESS;

        for(no_secteur; no_secteur<nb_secteur_a; no_secteur++)
        {
            no_site = lesSecteurA[no_secteur]->get_site()->get_no();
            //Si le site n'as pas encore �t� visit�
            if(sites_visites[no_site] == 0)
            {
                //on r�cup�re les secteurs du site
                find_secteur_from_site(no_site,lesSecteurA,nb_secteur_a,secteurs);
                //On r�cup�re l'�tat du site
                for(int i=0;i<3;i++)
                {
                    if(secteurs[i] == NULL) etat_site[i] = -1;
                    else etat_site[i] = secteurs[i]->get_porteuse();
                }



                //ESSAI permutation (1)       secteurs[0] <-> secteurs[1]
                fitness_tmp = test_permutation(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen,listeTabu,secteurs,0,1);
                if(fitness_tmp < ITERATE_FITNESS)
                {ITERATE_FITNESS = fitness_tmp; ITERATE_SITE = no_site; ITERATE_PERM = 1;}
                else if(fitness_tmp != 9999 && init_fitness< fitness_tmp && fitness_tmp <= BOF_FITNESS)
                {BOF_FITNESS = fitness_tmp; BOF_SITE = no_site; BOF_PERM = 1;}
                //ESSAI permutation (2)       secteurs[1] <-> secteurs[2]
                fitness_tmp = test_permutation(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen,listeTabu,secteurs,1,2);
                if(fitness_tmp < ITERATE_FITNESS)
                {ITERATE_FITNESS = fitness_tmp; ITERATE_SITE = no_site; ITERATE_PERM = 2;}
                else if(fitness_tmp != 9999 && init_fitness< fitness_tmp && fitness_tmp <= BOF_FITNESS)
                {BOF_FITNESS = fitness_tmp; BOF_SITE = no_site; BOF_PERM = 2;}
                //ESSAI permutation (3)       secteurs[0] <-> secteurs[2]
                fitness_tmp = test_permutation(stable,lesPTA, nb_tp_a, lesSecteurA, nb_secteur_a, no_scen,listeTabu,secteurs,0,2);
                if(fitness_tmp < ITERATE_FITNESS)
                {ITERATE_FITNESS = fitness_tmp; ITERATE_SITE = no_site; ITERATE_PERM = 3;}
                else if(fitness_tmp != 9999 && init_fitness< fitness_tmp && fitness_tmp <= BOF_FITNESS)
                {BOF_FITNESS = fitness_tmp; BOF_SITE = no_site; BOF_PERM = 3;}


                //On r�tablie les valeurs d'origine du site
                for(int i=0;i<3;i++)
                {
                    if(secteurs[i] != NULL)
                    {secteurs[i]->set_porteuse(etat_site[i]);}
                }

                sites_visites[no_site] = 1;
            }
        }


        update_Items_ListeTabuItems(listeTabu);


        //Si on a trouver une meilleur permutation on la fait
        if(ITERATE_PERM == -1)
        {
            ITERATE_FITNESS = BOF_FITNESS;
            ITERATE_PERM = BOF_PERM;
            ITERATE_SITE = BOF_SITE;
        }

        if(ITERATE_FITNESS < BEST_FITNESS)
        {
            for(int i = 0; i < nb_secteur_a; i++){BEST_CONF[i] = lesSecteurA[i]->get_porteuse();}
            BEST_FITNESS = ITERATE_FITNESS;
            cout << "Fitness tour  "<<iteration<<"  :  "<< BEST_FITNESS <<"   "<<BEST_FITNESS/NB_CLIENT*100<<"%  "<< endl;
        }

        find_secteur_from_site(ITERATE_SITE,lesSecteurA,nb_secteur_a,secteurs);
         switch(ITERATE_PERM){
             case 1 : index1=0;index2=1; break;
             case 2 : index1=1;index2=2; break;
             case 3 : index1=0;index2=2; break;
         }


        porteuse = secteurs[index1]->get_porteuse();
        secteurs[index1]->set_porteuse(secteurs[index2]->get_porteuse());
        secteurs[index2]->set_porteuse(porteuse);


        conf = new int[nb_secteur_a];
        for(int i = 0; i < nb_secteur_a; i++){conf[i] = lesSecteurA[i]->get_porteuse();}

        add_Item_ListeTabuItems(listeTabu,conf,DUREE_TABU);

    }



    delete secteurs;
    delete etat_site;
    delete sites_visites;
    delete_ListeTabuItems(listeTabu);


    cout<<endl<<"--------------------------"<<endl;
    cout<<endl<<endl<<endl<<endl;

    file_sortie.close();

}
