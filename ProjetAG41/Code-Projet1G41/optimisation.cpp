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
    TabuItem():no_site(-1),dureeTabu(0){}
    int no_site;
    int dureeTabu;
    };

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

///Impl�mentez votre m�thode ici
void optimisation::frequencyOptimization(char *nom, int stable,
										 pointTest** lesPTA, int nb_tp_a,
										 secteur** lesSecteurA, int nb_secteur_a, int no_scen){


}



























