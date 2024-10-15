# AJG2MID
Alberto José González (GB/GBC) to MIDI converter

This tool converts music from Game Boy and Game Boy Color games using Alberto José González (of Bit-Managers)'s sound engine to MIDI format.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex), followed by the offset to the first song pointer.

Examples:
* AJG2MID "Asterix (UE) (M5) [!].gb" 4 4000
* AJG2MID "Smurfs, The (UE) (V1.0) (M4) [!].gb" 7 4018
* AJG2MID "Spirou (E) (M4) [S][!].gb" 10 4062

This tool was particularly difficult to create, due to there being a number of differences, either minor or major, between how the format works in each game, although some games use the exact same commands. The code for this tool is rather messy, and a number of "hacks" were used to get it to get proper results for each game, although there might possibly still be certain songs that are still slightly problematic in a few games.
The included companion tool AJG2TXT was developed as a "prototype" and is very outdated compared to the MIDI converter.

Supported games:
  * Astérix
  * Astérix & Obélix
  * Baby Felix: Halloween
  * Bomb Jack
  * Bugs Bunny & Lola Bunny: Operation Carrots
  * Hugo 2/Hugo 2.5
  * Looney Tunes Collector: Martian Alert!
  * Looney Tunes Collector: Martian Revenge!
  * Lucky Luke
  * Die Maus
  * Otto's Ottifanten: Baby Bruno's Nightmare
  * Pop Up
  * Radikal Bikers (prototype)
  * Ronaldo V-Football
  * Sea Battle
  * The Smurfs
  * The Smurfs Nightmare
  * Spirou
  * Sylvester and Tweety: Breakfast on the Run
  * Tintin: Prisoners of the Sun
  * Tintin in Tibet
  * Turok: Battle of the Bionosaurs
  * Turok: Rage Wars
  * Turok 2: Seeds of Evil
  * Turok 3: Shadow of Oblivion
  * UEFA 2000
  * V-Rally: Championship Edition

To do:
  * Add support for other versions of the driver (NES, Game Gear, etc.)
