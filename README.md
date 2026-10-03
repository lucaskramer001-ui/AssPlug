# AssPlug – VST3 für macOS und Windows

Ein Regler, zwei Bänder, gegenläufig:

- Regler nach rechts: Mitten (Glocke 700 Hz, Q 1,5) lauter, Höhen (High Shelf ab 3 kHz) leiser
- Regler nach links: Höhen lauter, Mitten leiser
- Bereich ±18 dB, Doppelklick auf den Rubin = neutral

Die Filter entsprechen exakt dem Browser-Prototyp.

## 1. Schrift einfügen (einmalig)

Der Schriftzug nutzt die Google-Schrift **Rock Salt** (Apache-2.0-Lizenz, frei nutzbar).
Lade sie von https://fonts.google.com/specimen/Rock+Salt herunter und lege die Datei
`RockSalt-Regular.ttf` in den Ordner `Resources/`.
Ohne die Datei lässt sich das Plugin trotzdem bauen, dann mit einer Ersatzschrift.

## 2. Voraussetzungen

| | macOS | Windows |
|---|---|---|
| Compiler | Xcode oder Command Line Tools (`xcode-select --install`) | Visual Studio 2022 mit „Desktopentwicklung mit C++“ |
| CMake | ab 3.22 (`brew install cmake`) | ab 3.22 (cmake.org oder in Visual Studio enthalten) |
| Git | vorhanden | git-scm.com |

JUCE lädt CMake beim ersten Konfigurieren automatisch herunter (Internet nötig).

## 3. Bauen

Im Projektordner im Terminal bzw. in der „Developer PowerShell for VS 2022“:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Ergebnis: `build/AssPlug_artefacts/Release/VST3/AssPlug.vst3`
Auf dem Mac entsteht ein Universal Binary (Apple Silicon und Intel).

## 4. Installieren

- macOS: `AssPlug.vst3` nach `~/Library/Audio/Plug-Ins/VST3/` kopieren
- Windows: `AssPlug.vst3` nach `C:\Program Files\Common Files\VST3\` kopieren

Danach in der DAW nach neuen Plugins suchen lassen.

## Beide Plattformen ohne zweiten Rechner

Im Ordner `.github/workflows` liegt ein Build-Skript für GitHub Actions. Lädst du das
Projekt (inklusive Schriftdatei) in ein GitHub-Repository hoch, baut GitHub automatisch
die Mac- und die Windows-Version. Die fertigen Plugins findest du unter
„Actions“ → letzter Durchlauf → „Artifacts“.

## Hinweise zur Weitergabe

- **macOS:** Selbst gebaute Plugins laufen auf dem eigenen Mac sofort. Auf anderen Macs
  blockiert Gatekeeper unsignierte Plugins. Für eigene Tests hilft
  `xattr -cr ~/Library/Audio/Plug-Ins/VST3/AssPlug.vst3`. Für eine echte Veröffentlichung
  brauchst du eine Apple-Developer-ID zum Signieren und Notarisieren.
- **Lizenzen:** Prüfe vor einer Veröffentlichung die Lizenzbedingungen von JUCE
  (AGPLv3 oder JUCE-Lizenz) und des VST3-SDK von Steinberg.
