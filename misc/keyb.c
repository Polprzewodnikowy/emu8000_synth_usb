//***********************************************************************
// Plik: keyb.c 
//
// Zaawansowana obsługa przycisków i klawiatur 
// Wersja:    1.0
// Licencja:  GPL v2
// Autor:     Deucalion  
// Email:     deucalion#wp.pl
// Szczegóły: http://mikrokontrolery.blogspot.com/2011/04/jezyk-c-biblioteka-obsluga-klawiatury.html
//
//***********************************************************************

#include "inc/stm32f4xx.h"
#include "misc/keyb.h"
#include "config.h"

#define PERIOD_1S      100
#define PERIOD_750MS   75
#define PERIOD_500MS   50
#define PERIOD_100MS   10
#define PERIOD_30MS    3

// ***********************************************************************
static unsigned int keys;
// Wewnętrzna zmienna w której przechowywany jest stan klawiszy
// Zawartość tej zmiennej pobierana jest przez program za pomoca funkcji
//  GetKeys(); jeśli zmienna keycnt jest większa od zera.
// ***********************************************************************

// ***********************************************************************
static unsigned int keycnt;
// Wewnętrzna zmienna w której przechowywana jest licznik autorepetycji
// Zwiększany okresowo w momencie gdy są naciśnięte klawisze i ich stan
// nie zmienia się przez dłuższy czas. Zmniejszana podczas każdego
// wywołania funkcji GetKeys();
// ***********************************************************************

// ***********************************************************************
static unsigned int keytime;
// Wewnętrzna zmienna reprezentująca czas od ostatniego naciśnięcia
// klawiszy. Zwracana do programu za pomocą funkcji KeysTime();
// Jeśli nie ma wciśniętego żadnego klawisza, zmienna utrzymuje wartość 0;
// ***********************************************************************

// ***********************************************************************
static unsigned int prevKeybSt;
// Wewnętrzna zmienna pamiętająca stan klawiszy z poprzedniego wywołania
// funkcji KeybProc(). Służy do wykrycia zmiany stanu klawiatury
// ***********************************************************************

// ***********************************************************************
static unsigned int arTime;
// Wewnętrzna zmienna reprezentująca czas w którym ma nastąpić zwiekszenie
// licznika klawiatury. Zwiększana o odpowiedni czas zależny o czasów
// autorepetycji po każdym zwiekszeniu licznika klawiatury
// ***********************************************************************

// ***********************************************************************
static unsigned char arIndex ;
// Wewnętrzna zmienna indeksująca tablicę z kolejnymi czasami autorepetycji
// ***********************************************************************

// ***********************************************************************
static unsigned char keyblock;
// Wewnętrzna zmienna ustawiana na 1 funkcją KeybLock() lub ClrKeyb()
// z parametrem KBD_LOCK. Jeśli zmienna ma wartość 1 obsługa klawiatury
// zostaje zablokowana do momentu zwolnienia wszystkich klawiszy.   
// ***********************************************************************

// ***********************************************************************
// Domyślna tablica z kolejnymi czasami autorepetycji. Ostatnia pozycja różna
// od zera jest czasem autorepetycji nieskończonej. Ilość pozycji dowolna,
// ostatnia pozycja musi być równa 0;
static const unsigned short DefaultAutoRepeatTab[] =
{
	PERIOD_30MS,
	PERIOD_1S,
	PERIOD_500MS,
	PERIOD_500MS,
	PERIOD_500MS,
	PERIOD_500MS,
	PERIOD_100MS,
	0
};

static unsigned short 
	* volatile _art = (unsigned short *)DefaultAutoRepeatTab,
	* volatile art = (unsigned short *)DefaultAutoRepeatTab;

// ***********************************************************************
// Funkcja dostarczająca surowy stan klawiatury.
// Stan wysoki dla wcisniętych klawiszy. Jeśli funkcja KeybProc() 
// będzie wywoływana z przerwania to funkcja GetKeybSample() musi
// trwać jak najkrócej
// ***********************************************************************
unsigned int GetKeybSample(void)
{
	return (KEY1_bb|KEY2_bb);
}

// ***********************************************************************
// Funkcja wywoływana z jakiegos przerwania w tym przypadku co 10ms
// ***********************************************************************
void KeybProc(void)
{
	unsigned int realKeybSt;

	// Pobranie stanu klawiszy
	realKeybSt = GetKeybSample();

	// Sprawdzenie czy stan sie zmienił
	if(prevKeybSt != realKeybSt)
	{
		// Stan sie zmienił więc resetowanie klawiatury i wyjście z funkcji
		ClrKeyb(KBD_NOLOCK);
		return;
	}

	// Sprawdzenie czy brak naciśniętych klawiszy lub klawiatura zablokowana
	if(!realKeybSt || keyblock)
	{
		// Ponowne sprawdzenie czy brak nacisniętych klawiszy
		// Jeśli tak to odblokowanie klawiatury
		if(!realKeybSt) keyblock = 0;
			return;
	}
	// Zwiekszenie licznika czasu klawiatury
	keytime++;
	// Zachowanie stanu klawiszy
	keys = realKeybSt;

	// Obsługa autorepetycji
	// Sprawdzenie czy licznik czsu klawiatury osiągnął czas następnej
	// autorepetycji
	if(keytime >= arTime)
	{
		// Zwiększenie licznika autorepetycji
		keycnt++;
		// Obliczenie kolejnego czasu autorepetycji

		_art = art;
		if(_art[arIndex + 1]) arIndex++;
			arTime += _art[arIndex];
	}
}

// ***********************************************************************
// Funkcja zwraca stan klawiszy do programu jeśli licznik autorepetycji
// rózny od zera
// ***********************************************************************
unsigned int GetKeys(void)
{
	if(keycnt)
	{
		keycnt--;		// Jeśli funkcja KeybProc() będzie wywoływana
						// z przerwania to ta instrukcja musi być wykonana
						// atomowo.
		return keys ;
	}
	return 0;
}

// ***********************************************************************
// Funkcja zwraca czas wciskania aktualnej kombinacji klawiszy
// ***********************************************************************
unsigned int KeysTime(void)
{
	return keytime;
}

// ***********************************************************************
// Funkcja zwraca stan klawiszy zgodnie z podana maską jako argument funkcji
// ***********************************************************************
unsigned int IsKeyPressed(unsigned int mask)
{
	return keys & mask;
}

// ***********************************************************************
// Funkcja zwraca stan klawiszy zgodnie z ustawiona maską podana jako
// argument funkcji, jest brany pod uwage licznik autorepetycji
// Ale pobranie stanu klawiwszy nie zminiejsza licznika autorepetycji
// ***********************************************************************
unsigned int IsKey(unsigned int mask)
{
	if(keycnt)
	{
		return keys & mask ;
	}
	return 0;
}

// ***********************************************************************
// Funkcja resetuje stan klawiatury. Jako parametr należy podać stałą
// KBD_LOCK lub KBD_NOLOCK, które odpowiednio blokują lub nie klawiaturę
// Jeśli funkcja KeybProc() będzie wywoływana z przerwania to funkcja
// ClrKeyb() musi być wykonana atomowo.
// ***********************************************************************
void ClrKeyb(int lock)
{
	prevKeybSt = GetKeybSample();
	keys = 0;
	keytime = 0;
	keycnt = 0;
	arIndex = 0;
	arTime = _art[0];
	if(lock) keyblock = 1;
}

// ***********************************************************************
// Funkcja blokuje klawiaturę. Odblokowanie następuje po zwolnieniu
// wszystkich klawiszy.
// ***********************************************************************
void
KeybLock(void)
{
	keyblock = 1;
}

// ***********************************************************************
// Funkcja podmienia tablicę z międzyczasami autorepetycji. Nowa tablica
// powinna być zgodna z wcześniej opisanym formatem
// ***********************************************************************
void
KeybSetAutoRepeatTimes(unsigned short *AutoRepeatTab)
{
	if(AutoRepeatTab == KBD_DEFAULT_ART)
		art = (unsigned short *)DefaultAutoRepeatTab;
	else
		art = AutoRepeatTab;
}
