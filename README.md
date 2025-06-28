# CSOPESYMO_JIRO

Members: Jiro Phoenix Lim

How to run:
-Download the zip file
-Extract it
-Open the CSOPESY.sln (Solution File) in Visual Studio 2022
-Press local windows debugger

A main menu console for recognizing the following commands:
“initialize” – initialize the processor configuration of the application. This must be called
before any other command could be recognized, aside from “exit”.
“exit” – terminates the console.
“screen” – see additional details.
“scheduler-start” (formerly scheduler-test) – continuously generates a batch of dummy
processes for the CPU scheduler. Each process is accessible via the “screen” command.
“scheduler-stop” – stops generating dummy processes.
“report-util” – for generating CPU utilization report. See additional details.
process-smi” – Prints a simple information about the process. The process contains
dummy instructions that the CPU executes in the background. Whenever the user types “process-smi”,
it provides the updated details and accompanying logs from the print instructions.
