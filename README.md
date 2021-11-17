A simple application to log key presses in MacOS.

## Setup

Before starting:

* Set the environment variable `KEYLOGGER_PATH` with the path where you want to log data to.

Suggestion, in your `.bash_profile`, add:

    export KEYLOGGER_PATH="$HOME/var/"

## Running

### Backend

The backend code is under `/daemon`. To build it, run:

    make

To start running:

    make run

**NOTE:** This app requires accessibility privileges and there seems to be no way to pre-grant that to the binary, so the first time it will run, it will ask for permission.

### Frontend

Once it starts logging data to `KEYLOGGER_PATH`, you can start the UI:

    npm start

## Scheduling

TDB
