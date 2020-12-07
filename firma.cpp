/*
 * @file firma.cpp
 * @author xpolic05
 * @author xdubec00
 */

#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include "simlib.h"
#include <cstring>

using namespace std;

// Konštanty
const int MINUTA = 60;
const int HODINA = 60 * MINUTA;
const int DEN = 24 * HODINA;
const int ROK = 365 * DEN;

// počet zamestnancov
const int POCET_ZAMESTNANCU = 3;

// Simulačná doba v simulačnom čase
double DOBA_SIMULACIE = 1 * DEN;

// zariadenia
Facility f_peeler;
Facility f_slicer;
Facility f_washer;
Facility f_boiler[2];

// queue pre surove brambory
Queue q_surove_kusy;

// je smena alebo nie?
bool je_den = false;

int pocet_pracujicich_zamestnancu = 0;

int pocet_in_brambor = 0;
float pocet_out_brambor = 0.0;
float pocet_out_odpad = 0.0;

class SUROVY_KUS : public Process {

  void Behavior() {
  	// pridá do fronty tento process
    q_surove_kusy.Insert(this);
    // stopnutie transakcie
    Passivate();
    // ukončenie transakcie
    Cancel();
  }

 public:

  SUROVY_KUS() { 

  	Activate(); 
  }
};

// Proces výroby
class ProcesVyroby : public Process {

	void Behavior() {

			float multiplier = 1.0;
			float odpad = 0;
			int zemiaky = 0;

			// ak je aspon 4kg vo fronte tak sa vyprázdni a začína výroba
			while (q_surove_kusy.Length() < 4) {
				Wait (1);
			}

			// zistí koľko kilo berie
			zemiaky += q_surove_kusy.Length();
			multiplier *= q_surove_kusy.Length();
			q_surove_kusy.clear();
			
			//peeling
			Seize(f_peeler);
			Wait (4*MINUTA);
			Release(f_peeler);

			// 15% odpad - supka
			odpad += multiplier;
			multiplier *= Uniform(0.80,0.90);
			odpad -= multiplier;

			//inspection
			Wait (4*MINUTA);

			// 10% odpad
			odpad += multiplier;
			multiplier *= Uniform(0.85,1);
			odpad -= multiplier;

			//slicer
			Seize(f_slicer);
			Wait (4*MINUTA);
			Release(f_slicer);

			//washing
			Seize(f_washer);
			Wait (4*MINUTA);
			Release(f_washer);

			// drainage belt
			Wait (5*MINUTA);

			// 60% sa odpari
			multiplier *= Uniform(0.35,0.45);

			//cooking
			if (!f_boiler[1].Busy()) {
				Seize(f_boiler[1]);
				Wait (8*MINUTA);
				Release(f_boiler[1]);
			} else {
				Seize(f_boiler[0]);
				Wait (8*MINUTA);
				Release(f_boiler[0]);
			}

			// inspection
			Wait (5 * MINUTA);

			// 10% odpad
			odpad += multiplier;
			multiplier *= Uniform(0.85,1);
			odpad -= multiplier;

			// packing
			Wait (1*MINUTA);

			// FINISHED PRODUCTS ***************************************************
			
			// vyrobene chipsy udane v percentach z kila
			pocet_out_brambor += multiplier;

			pocet_out_odpad += odpad;

			pocet_in_brambor += zemiaky;

			// výpis
			unsigned long long t = Time;
	      	cout << setw(3) << (int)t / ROK << "r, " << setw(3)
	        	<< (int)(t % ROK) / DEN << "d,  " << setw(2)
	            << (int)(t % DEN) / HODINA << ":" << setw(2)
	            << (int)(t % HODINA) / MINUTA << ":" << setw(2) << (t % MINUTA)
	            << (" | spotrebovane kg: " + to_string(pocet_in_brambor))
	            << (" | vyrobene kg: " + to_string(pocet_out_brambor))
	            << (" | odpad kg: " + to_string(pocet_out_odpad))
	            << " |" << endl;
	}

};

// generátor zemiakov pre jeden deň
class Gen_Brambor : public Event {

	// vygenerované zemiaky za deň
	int vygen_brambor;

	void Behavior() {

		if(je_den) {
			(new SUROVY_KUS)->Activate();
			vygen_brambor++;
			Activate(Time + Uniform(1 , 1 * MINUTA));
		}
	}

	public:
		Gen_Brambor(){

			vygen_brambor = 0;

			Activate();
		}
};

// Posúva linku každé 4 minúty
class Posun_Linky : public Event {

	// pocet posunov za deň
	int pocet_iteracii;

	void Behavior() {

		if(je_den) {
			(new ProcesVyroby)->Activate();
			pocet_iteracii++;
			Activate(Time + (4 * MINUTA));
		}
	}

	public:
		Posun_Linky(){

			pocet_iteracii = 0;

			Activate();
		}
};

// striedanie smien
class GEN_DEN : public Event {

	void Behavior() {

		if(je_den) {
			je_den = false;
			Activate(Time + (17*HODINA));

		// spustenie generatoru brambor ak zacne den
		} else {
			je_den = true;
			(new Gen_Brambor())->Activate();
			(new Posun_Linky())->Activate();

			Activate(Time + (7*HODINA));
		}
	}
};

/*
 * Riadiaca funkcia main. Nastavuje a riadi simuláciu.
*/
int main(int argc, char **argv) {

	RandomSeed(time(NULL));


	if (argc == 3) {
		try {
			cout << "try " << atof(argv[1]) << " : " << argv[2] << endl;
			if (strcmp(argv[2],"DEN") == 0)
				cout << "here" << endl;
				DOBA_SIMULACIE = atof(argv[1]) * DEN;
			if (strcmp(argv[2],"MINUTA") == 0)
				DOBA_SIMULACIE = atof(argv[1]) * MINUTA;
			if (strcmp(argv[2],"HODINA") == 0)
				DOBA_SIMULACIE = atof(argv[1]) * HODINA;
		} catch (int e) {}
	}
	if (argc == 2 || argc > 3) {
		cout << "/=================================| HELP |=================================\\" << endl
			 << "|" << "argumenty: ./firma (double) (DEN/HODINA/MINUTA)                           |" << endl             
			 << "|" << "Zadane cislo v prvom argumente udava pocet v jednotke z druheho argumentu.|" << endl
			 << "****************************************************************************" << endl;
		return 0;
	}

	// Inicializácia experimentu
	Init(6*HODINA, DOBA_SIMULACIE + 6 * HODINA);

	// Nastavenie nazvov strojov
	f_peeler.SetName("Peeler");

	f_slicer.SetName("Slicer");

	f_washer.SetName("Drum Washer");

	for (int i = 0; i < 2; i++) {
		f_boiler[i].SetName("Boiler");
	}

	// Aktivácia smeny.
	(new GEN_DEN)->Activate();

	// Štart simulácie
	Run();
} 
 
