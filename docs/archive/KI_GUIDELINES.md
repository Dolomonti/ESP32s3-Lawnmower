# 🤖 KI-Agent Richtlinien & Entwicklungs-Tagebuch

> **Dokument erstellt:** 2026-03-15  
> **Zweck:** Zentrale Dokumentations- und Archivierungsrichtlinien für alle KI-gestützten Code-Änderungen

---

## 📜 PHILOSOPHIE: "Diario - Das Tagebuch"

Jede Code-Änderung ist eine **TAT**, die dokumentiert werden muss. Dieser Ordner (`docs/archive/`) dient als **zeitlich korrektes Tagebuch** aller Entwicklungsschritte.

> **Golden Rule:** *"Wer code ändert, ohne es zu dokumentieren, hat es nicht getan."*

---

## 📝 PFLICHTBEI JEDER ÄNDERUNG

### Schritt 1: Ordnerbaum aktualisieren
Wenn du neue Dateien erstellst, Ordner verschiebst oder die Struktur änderst:

```bash
# Immer den aktuellen Stand dokumentieren
- Erstelle/Unterbreche: docs/archive/tree/YYYY-MM-DD_tree.md
- Liste ALLE geänderten/verschobenen/neuen Dateien auf
```

### Schritt 2: To-Do-Liste aktualisieren
Erstelle einen neuen Eintrag in der aktuellen To-Do-Datei:

**Template für neue Tasks:**
```markdown
### 🆕 [YYYY-MM-DD HH:MM] Task: [Kurzbeschreibung]

**Status:** ✅ Erledigt / 🔄 In Arbeit / ⬜ Offen  
**Agent:** [KI-Name/Version]  
**Dateien betroffen:** `[datei1.cpp]`, `[datei2.h]`  

**Was wurde getan:**
- Detaillierte Beschreibung der Änderung
- Zeilennummern wenn relevant (Zeile 145-167)
- Neue Funktionen/Variablen dokumentieren

**Warum wurde es getan:**
- Technische Begründung
- Fehlerbeschreibung die behoben wurde
- Architektur-Verbesserung

**Verifikation:**
- [ ] Build erfolgreich (`pio run`)
- [ ] Tests bestanden
- [ ] Hardware-Test (falls relevant)

**Commit-Hash:** `abc1234`
```

### Schritt 3: Bei neuen Tag (neuer Tag = neuer Eintrag)
Wenn ein Tag noch keinen Eintrag hat:

```markdown
## 📅 TAG: [YYYY-MM-DD]

### Zusammenfassung
- [Anzahl] Tasks erledigt
- [Anzahl] Dateien geändert
- Hauptfokus: [Beschreibung]

### Details:
[Siehe einzelne Task-Einträge oben]
```

---

## 📂 ORDNERSTRUKTUR (Dieser Archive-Ordner)

```
docs/archive/
├── KI_GUIDELINES.md          ← DU BIST HIER (Regelwerk)
├── todo-history/               ← ALTE To-Do-Listen (Nur lesen!)
│   ├── To-do1.md              ← Phase 1 (veraltet)
│   ├── To-do2.md              ← Phase 2 (veraltet)
│   └── to-do3.md              ← Aktuellste Version
├── tree/                       ← Ordnerbaum-Snapshots
│   ├── 2026-03-13_tree.md
│   └── 2026-03-15_tree.md
├── changes/                    ← Detaillierte Change-Logs
│   └── 2026-03-15_pinout-fix.md
└── decisions/                  ← Architektur-Entscheidungen
    └── 2026-03-13_blade-safety-standby.md
```

---

## ⚠️ WICHTIGE REGELN

1. **NIE die alten To-do-Dateien in `todo-history/` bearbeiten!**
   - Diese sind als "Dark History" archiviert
   - Neue Tasks IMMER in der aktuellen Datei oder neuem Tageseintrag

2. **PINOUT-Änderungen = KRITISCH**
   - Jede GPIO-Änderung muss in ALLE Dokumentationen:
     - `AGENTS.md`
     - `docs/TECHNICAL_MANUAL.md`
     - `docs/archive/changes/[datum]_pinout-*.md`

3. **Code-Bugs gefunden?**
   - Sofach dokumentieren mit `BUG:`-Präfix
   - Beispiel: `BUG: BLADE_UNIT_PIN und DRIVE_UNIT_PIN beide auf GPIO 20 definiert`

4. **OTA Password & Partition Scheme**
   - Sind SENSIBLE/TECHNISCHE Details
   - Müssen konsistent zwischen `platformio.ini`, `AGENTS.md` und `TECHNICAL_MANUAL.md` sein

---

## 🔍 HISTORISCHE TO-DO-LISTEN (Index)

| Datei | Zeitraum | Status | Enthält |
|-------|----------|--------|---------|
| `todo-history/To-do1.md` | 2026-03-13 | Archiviert | Phase 0-4 Planung |
| `todo-history/To-do2.md` | 2026-03-13 | Archiviert | Kritische Fixes |
| `todo-history/to-do3.md` | 2026-03-13 | Archiviert | Code-Qualitäts-Audit |

**Aktuelle Arbeit:** Siehe neuesten Eintrag in diesem Ordner oder `AGENTS.md`

---

## 🏗️ ARCHITEKTUR-ENTSCHEIDUNGEN (Log)

| Datum | Entscheidung | Begründung | Status |
|-------|--------------|------------|--------|
| 2026-03-13 | Blade-Safety Bypass auf Standby | User-Anweisung | ⏸️ Pausiert |
| 2026-03-15 | Pinout-Dokumentation zentralisieren | Inkonsistenzen gefunden | ✅ In Arbeit |

---

## 📞 KONTAKT & VERANTWORTUNG

**Projekt:** ESP32-S3 Lawnmower  
**Eigentümer:** Markus Montagner  
**KI-Archiv-Verwalter:** Jede KI-Instanz (Rotation)

---

> *"Dieses Dokument ist lebendig. Jede KI-Instanz trägt zur Geschichte bei."*
