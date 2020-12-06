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
const double SIMULACNI_DOBA = 1 * DEN;

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
float pocet_out_brambor = 0.0;
float pocet_out_odpad = 0.0;

class SUROVY_KUS : public Process {

  void Behavior() {
    q_surove_kusy.Insert(this);  // přidej do fronty čekajících surových kusů

    Passivate();

    Cancel();  // transakce se ukončí
  }

 public:

  SUROVY_KUS() { 

  	Activate(); 
  }
};

class OneKGofPotatoes : public Process {


	void Behavior() {

		if(je_den) {

			float multiplier = 1.0;
			float odpad = 0;
			int zemiaky = 0;

			while (q_surove_kusy.Length() < 4) {
				Wait (1);
			}

			zemiaky += q_surove_kusy.Length();
			multiplier *= q_surove_kusy.Length();
			q_surove_kusy.clear();
			
			//peeling
			Seize(f_peeler);
			Wait (4*MINUTA);
			Release(f_peeler);

			(new OneKGofPotatoes) -> Activate();

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
			// 60% sa odpari
			multiplier *= Uniform(0.35,0.45);

			// drainage belt
			Wait (5*MINUTA);

			// inspection
			Wait (5*MINUTA);

			// 10% odpad
			odpad += multiplier;
			multiplier *= Uniform(0.85,1);
			odpad -= multiplier;
			
			// vyrobene chipsy udane v percentach z kila
			pocet_out_brambor += multiplier;

			pocet_out_odpad += odpad;

			pocet_in_brambor += zemiaky;

			// debug
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
	Passivate();
	}

};

//* generator brambor pre jeden den
class Gen_Brambor : public Event {

	//* vygenerovanych brambor za den
	int vygen_brambor;

	void Behavior() {

		if(je_den) {
			(new SUROVY_KUS)->Activate();
			vygen_brambor++;
			Activate(Time + Uniform(1 * MINUTA, 2 * MINUTA));
		}
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
			cout << " ******************************************** " << je_den << " ******************************************** " << endl;
			Activate(Time + (16*HODINA));

		//* spustenie generatoru brambor ak zacne den
		} else {
			je_den = true;
			cout << " ******************************************** " << je_den << " ******************************************** " << endl;
			(new Gen_Brambor())->Activate();

			Activate(Time + (8*HODINA));
		}
	}
};

// Tato třída představuje událost, zajišťující generování zaměstnanců.
class GEN_ZAMESTNANCU : public Event {
  void Behavior() {
    for (int i = 0; i < POCET_ZAMESTNANCU; i++) {
      // Vygenerované zaměstnance umístíme do fronty zaměstnanců.%
      //q_zamestnanci.Insert(this);
    }
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

	(new OneKGofPotatoes) -> Activate();

	// Generuj zaměstnance.
	//(new GEN_ZAMESTNANCU)->Activate();

	// Start simulace.
	Run();

} 
 
