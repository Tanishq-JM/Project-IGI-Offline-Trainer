# Security Policy

## Supported Version

Only the latest tagged release is supported.

## Reporting

Open a GitHub issue for ordinary bugs. For a vulnerability that could expose private data, create unsafe writes, or affect systems beyond the authorized offline game process, use GitHub's private security-advisory feature rather than posting exploit details publicly.

## Project Boundaries

Reports requesting online-game support, anti-cheat bypass, concealment, DRM bypass, credential access, injection, or unrelated process manipulation are outside this project's scope.

## Data Handling

The application has no telemetry or remote logging. It reads and conditionally writes only to the locally running `igi.exe` process. Correction counters exist only in memory for the current trainer session.
