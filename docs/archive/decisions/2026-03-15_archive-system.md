# 📋 Architektur-Entscheidung: KI-Archiv-System (Diario)

> **Datum:** 2026-03-15  
> **Entscheidungsträger:** Markus Montagner (User) + Kimi Code CLI  
> **Status:** ✅ Implementiert

---

## 🎯 PROBLEMSTELLUNG

**Beobachtung:**
- Mehrere To-Do-Listen (`To-do1.md`, `To-do2.md`, `to-do3.md`) mit überlappenden Inhalten
- Inkonsistenzen zwischen Dokumentation und Code (GPIO-Pinout)
- Keine zentrale Historie für zukünftige KI-Instanzen
- "Vibe-Coder-Entropie" - unklare Verantwortlichkeiten

**Anforderung:**
Ein System, das:
1. Alle historischen To-Do-Listen archiviert
2. Jedes Update mit Zeitstempel und Begründung dokumentiert
3. Zukünftigen KI-Agents Kontext bietet
4. Als "Tagebuch" (Diario) der Entwicklung dient

---

## 💡 ENTSCHEIDUNG

### Etablierung von `docs/archive/` als "Dark History" + Entwicklungs-Tagebuch

**Struktur:**
```
docs/archive/
├── KI_GUIDELINES.md          ← Richtlinien für KI-Agents
├── todo-history/              ← Alte To-Do-Listen (read-only)
│   ├── To-do1.md
│   ├── To-do2.md
│   └── to-do3.md
├── tree/                      ← Ordnerbaum-Snapshots
├── changes/                   ← Detaillierte Change-Logs
└── decisions/                 ← Architektur-Entscheidungen
```

**Pflicht für KI-Agents:**
1. Bei jeder Änderung: `tree/YYYY-MM-DD_tree.md` aktualisieren
2. Bei jeder Änderung: Eintrag in aktueller To-Do/Changelog mit:
   - Zeitstempel
   - Was wurde getan
   - Warum wurde es getan
   - Verifikation (Build-Status)

---

## 📝 IMPLEMENTIERUNG

| Bestandteil | Status | Datei |
|-------------|--------|-------|
| Richtlinien | ✅ | `docs/archive/KI_GUIDELINES.md` |
| To-Do-Archiv | ✅ | `docs/archive/todo-history/*.md` |
| Baum-Snapshot | ✅ | `docs/archive/tree/2026-03-15_tree.md` |
| Change-Log | ✅ | `docs/archive/changes/2026-03-15_pinout-correction.md` |
| Diese Entscheidung | ✅ | `docs/archive/decisions/2026-03-15_archive-system.md` |

---

## ✅ AKZEPTANZKRITERIEN

- [x] Alle alten To-Do-Dateien archiviert (nicht gelöscht)
- [x] AGENTS.md aktualisiert mit Hinweis auf KI_GUIDELINES.md
- [x] Konsistente Pinout-Dokumentation über alle MD-Dateien
- [x] Bekannter Code-Bug (BLADE_UNIT_PIN) dokumentiert

---

## 🔮 ZUKÜNFTIGE ARBEIT

**Offene Punkte:**
1. Code-Fix für `BLADE_UNIT_PIN` (sollte 19 statt 20 sein)
2. Weitere historische Dokumentation migrieren
3. Automatische Verifikation der Pinout-Konsistenz?

---

## 📎 VERKNÜPFUNGEN

- Übergeordnet: `docs/archive/KI_GUIDELINES.md`
- Verwandt: `docs/archive/changes/2026-03-15_pinout-correction.md`
