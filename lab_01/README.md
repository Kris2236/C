Biblioteka umożliwia:

  - Utworzenie tablicy wskaźników (tablicy głównej) — w tej tablicy będą przechowywane wskaźniki na wiersze zmergowanych plików — pierwszy element tablicy głównej zawiera wykaz wierszy pierwszej pary zmergowanych plików, drugi element dla drugiej pary, itd. Pojedynczy blok wierszy (element wskazywany z tablicy głównej), to tablica wskaźników na poszczególne wiersze w zmergowanym pliku
  - Definiowanie sekwencji par plików
  - Przeprowadzenie zmergowania (dla każdego elementu sekwencji) oraz zapisanie wyniku zmergowania do pliku tymczasowego
  - Utworzenie, na podstawie zawartość pliku tymczasowego, bloku wierszy — tablicy wskaźników na wiersze, ustawienie w tablicy głównej (wskaźników) wskazania na ten blok; na końcu, funkcja powinna zwrócić indeks elementu tablicy (głównej), który zawiera wskazanie na utworzony blok
  - Zwrócenie informacji o ilości wierszy w danym bloku wierszy
  - Usunięcie, z pamięci, bloku (wierszy) o zadanym indeksie
  - Usunięcie, z pamięci, określonego wiersza dla podanego bloku wierszy
  - Wypisanie zmergowanych plików, z uwzględnieniem wcześniejszych usuniętych bloków wierszy / wierszy
