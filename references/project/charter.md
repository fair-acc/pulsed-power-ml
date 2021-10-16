# Project Charter

## Business background

* Who is the client, what business domain the client is in.
  * [GSI Helmholtzzentrum für Schwerionenforschung GmbH](https://www.gsi.de/forschungbeschleuniger/fair)
* What business problems are we trying to address?
  * Energymanagement

## Scope
* What data science solutions are we trying to build?
  * We are building a monitoring system for energy consuming devices. 
  * Basis for our work will be an experimental setup using hardware provided by GSI/FAIR with the central piece [Picoscope 4000A](https://www.picotech.com/oscilloscope/4000/picoscope-4000-series)
  * Those devices will be classified by energy consumption parameters.
  * Energy consumption will be plotted and annotated. 
* What will we do?
  * Create testfield based around the **Picoscope 4000A** publishing data via ZMQ stream (GNU Radio)
  * Train classification model
  * Annotate and visualize data
* How is it going to be consumed by the customer?
  * The data will be displayed via [HoloViews](https://holoviews.org/) dashboard

## Personnel
* Who are on this project:
	* infoteam:
        * Heinesch, Roland (Roland.Heinesch@infoteam.de)
        * Förstel, Stefan (Stefan.Foerstel@infoteam.de)
        * Kittler, Thomas (Thomas.Kittler@infoteam.de)
        * Signoriello, Stefano (Stefano.Signoriello@infoteam.de)
        * Neumann, Christoph (Christoph.Neumann@infoteam.de)
	* GSI/FAIR:
		* Steinhagen, Ralph (R.Steinhagen@gsi.de)
		* Krimm, Alexander (A.Krimm@gsi.de)

## Metrics
* Classification is working with x% accuracy
* tbd

## Plan
* Phases (milestones), timeline, short description of what we'll do in each phase.
  * Phase 1a (depending on when we can get the complete hardware):
    * Testfield works and streams live data via **Picoscope4000a**
  * Phase 1b ():
    * Fix the custom codeblock "Picoscope4000a" to work with GNURadio 3.8+
  * Phase 2 (Build pipeline with synthetic data till Phase 1a is done):
    * Classify streamed / received data
  * Phase 3:
    * Display annotated data in dashboard

Architektur (Live):
* Generation von Daten --> Pub via ZMQ
* Sub via ZMQ --> Bekommen Live Daten
* Klassifikation der Live Daten durch Modell
* Annotation der Live Daten + Visualisierung

Architektur (Training):
* Generation von Daten --> Pub via ZMQ
* Sub via ZMQ --> Rohdaten + Labels
* Training des Modells


Pakete:
1. Thomas + Christoph: ZMQ Anbindung an VM, schreibe synthetische Trainingsdaten für Pipeline-Basis
2. Christoph: Fix the custom codeblock "Picoscope4000a" to work with GNURadio 3.9
3. Stefano + Thomas: Aufbauend auf 1.

## Architecture
* Data
  * Datatypes are tbd
* Data movement:
  * Data will be streamed via "ZMQ Pub Sink" and subscribed to
  * For training this data will be written to disk

## Communication
* Weekly meetings, Monday 13:00 - 13:30
  * Via [Matrix](https://matrix.org/).
* Who are the contact persons on both sides?
  * infoteam
    * Roland Heinesch
  * GSI / FAIR
    * Ralph Steinhagen