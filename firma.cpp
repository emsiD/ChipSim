/*
 * @file firmaHridele.cpp
 * @author Jan Kotas<xkotas07@stud.fit.vutbr.cz>
 * @author Matúš Burzala<xburza00@stud.fit.vutbr.cz>
 */

#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include "simlib.h"

using namespace std;

// Důležité konstanty pro efektivnejsi práci s časem a pro lepší abstrakci.
const int MINUTA = 60;
const int HODINA = 60 * MINUTA;
const int DEN = 24 * HODINA;
const int ROK = 365 * DEN;

//* pocet zamestnancov
const int POCET_ZAMESTNANCU = 3;

// Simulační doba představuje délku simulace v simulačním čase.
const double SIMULACNI_DOBA = 1 * ROK;

// zariadenia
Facility f_peeler;
Facility f_slicer;
Facility f_washer;
Facility f_boiler[2];

// queue pre zamestnancov
Queue q_zamestnanci;
// queue pre surove brambory
Queue q_surove_kusy;

//stridani dne a noci
bool je_den = false;

int pocet_pracujicich_zamestnancu = 0;

int pocet_in_brambor = 0;
int pocet_out_brambor = 0;

class OneKGofPotatoes : public Process {


	void Behavior() {

		pocet_in_brambor++;

		float multiplier = 1.0;
		
		//peeling
		Seize(f_peeler);
		Wait (4*MINUTA);
		Release(f_peeler);

		//inspection
		Wait (4*MINUTA);

		// 10% odpad
		multiplier = multiplier * 0.9;

		//slicer
		Seize(f_slicer);
		Wait (4*MINUTA);
		Release(f_slicer);

		//washing
		Seize(f_washer);
		Wait (4*MINUTA);
		Release(f_washer);

		//cooking
		Seize(f_boiler[1]);
		Wait (8*MINUTA);
		Release(f_boiler[1]);

		// 60% sa odpari
		multiplier = multiplier * 0.4;

		// drainage belt
		Wait (5*MINUTA);

		// inspection
		Wait (5*MINUTA);

		// 10% odpad
		multiplier = multiplier * 0.9;
		
		// vyrobene chipsy udane v percentach z kila
		pocet_out_brambor = multiplier;

		// debug
		unsigned long long t = Time;
      	cout << setw(3) << (int)t / ROK << "r, " << setw(3)
           << (int)(t % ROK) / DEN << "d,  " << setw(2)
           << (int)(t % DEN) / HODINA << ":" << setw(2)
           << (int)(t % HODINA) / MINUTA << ":" << setw(2) << (t % MINUTA)
           << (" -- vyrobene kg = " + to_string(multiplier))
           << endl;
	}

};

//* generator brambor pre jeden den
class Gen_Brambor : public Event {

	//* vygenerovanych brambor za den
	int vygen_brambor;

	void Behavior() {

		if(je_den) {
			(new OneKGofPotatoes)->Activate();
			vygen_brambor++;

			Activate(Time + Uniform(1, 3));
		}
		//printf("%d - vygen_brambor\n", vygen_brambor);
	}

	public:
		Gen_Brambor(){

			vygen_brambor = 0;

			Activate();
		}
};

//* striedanie dna a noci
class GEN_DEN : public Event {

	void Behavior() {

		if(je_den) {
			je_den = false;
			printf("%d - vygen_brambor\n", pocet_in_brambor);
			printf("**************** noc *******************\n");
			Activate(Time + (8*HODINA));

		//* spustenie generatoru brambor ak zacne den
		} else {
			je_den = true;
			printf("**************** den *******************\n");
			(new Gen_Brambor())->Activate();
			Activate(Time + (16*HODINA));
		}
	}
};

// Tato třída představuje událost, zajišťující generování zaměstnanců.
class GEN_ZAMESTNANCU : public Event {
  void Behavior() {
    for (int i = 0; i < POCET_ZAMESTNANCU; i++) {
      // Vygenerované zaměstnance umístíme do fronty zaměstnanců.%
      //q_zamestnanci.Insert(this);
      printf(" +1 zamestnanec\n");
    }
    printf("%d - zamestnanci\n", q_zamestnanci.Length());
  }
};

/*
 * Řídící funkce main. Nastavuje a aktivuje simulaci.
*/
int main(int argc, char **argv) {

	RandomSeed(time(NULL));

	// Inicializace experimentu.
	Init(0, SIMULACNI_DOBA);

	// Nastavenie nazvov strojov
	f_peeler.SetName("Peeler");

	f_slicer.SetName("Slicer");

	f_washer.SetName("Drum Washer");

	for (int i = 0; i < 2; i++) {
		f_boiler[i].SetName("Boiler");
	}

	// Aktivace dne.
	(new GEN_DEN)->Activate();

	// Generuj zaměstnance.
	//(new GEN_ZAMESTNANCU)->Activate();

	// Aktivace periodického generování surových kusů.
	(new Gen_Brambor)->Activate();

	// Start simulace.
	Run();

} 
 
