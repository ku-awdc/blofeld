---
title: Brainstorming modelling framework
subtitle: 
author: 
  - Mossa
theme: metropolis
institute: IVH
aspectratio: 1610
date: 2021-04-15
linkcolor: red
urlcolor: CornflowerBlue
header-includes:
    # - \RequirePackage{fira} # you've got to install this before the font works
    - \usepackage{fontspec}
    - \setmonofont[Contextuals={Alternate}]{Fira Code}
    - \makeatletter
    - \def\verbatim@nolig@list{}
    - \makeatother
---

<!-- markdownlint-disable-file MD025 -->

# Checklist

- [ ] Timestep synchronization between modules

## Background

It is easier to constraint opinions, when it has to fit within the frames of
a Beamer-presentation.

Other documents to read first:

* [Model Framework Project](documentation/Model%20Framework%20Project%20v5.md)
* [Model Requirement Sheet](documentation/Model%20Requirement%20Sheet%20v4.md)

### Approach here

We decided to take a **top-down** design approach, thus we'll start by listing
a general overview and then cut down further.

There are some (three) sub-models congruently facilitating the scenario simulation.

* There should be a fairly permissive interface for the actual models.
* An interface between sub-models.
* Interface for data-sources. These should be fairly permissive.

## Overview

Coupling between R and *compiled language[^compiled_lang]*: There must be
a way to define code in R and have that code run in the **compiled language**.

This means that every requirement we are able to formulate within a compiled language
has to be to be setup and piped through the compiled language.

[^compiled_lang]: Rust/Fortran/C/C++.

## General process modules in the framework

These are:

- Population
- Disease Model
- Regulator / Authorities

Stored state within these models should comply with the interface

## Latency between R and Rust

* Serialization/Deserialization between R & Rust is a way to start the process,
  but is not a sustainable solution forward. Pointers has to be passed back and
  forth.

* Provide what is allowed to be changed from the R-side, thus we can ensure that
  costly inquiries to the *population* can be facilitated through the Rust.

* [ ] Is it possible to JIT-compile R code that then can be embedded in Rust
  and figure into the compilation.

# Storage

During simulation, actually storing the model state in a hierarchical data
structure is not a good idea. Instead, we should aim for cache-friendly
organisation of the data.

This is where an ECS comes in.

However, there is a storage component that have to be considered, where the trade-off
is between faster insertion/deletion of new entities, or updating existing entities.

# Recording

# Architectural idioms

For now, the framework should not provide any default implementations of anything
or templates that must be followed.

1. Constraints -- traits and contracts
2. Implementation of the framework
3. Default implementations

#

<!-- Based on a meeting on 2021-04-16 -->

There must be several (disease) spread models, several regulatory modules,
and several populations. The composability of each should be considered in the
modelling framework.

This necessitates simulation *stages* that would necessitates and end of a stage
where these module outcomes are compounded and affected.

# Parallel scenario run

* Define a scenario repetitions/iterations $N$ and duration in timepoints $M$.

This means that we parallelise over the repetitions $N$. E.g. we'd have a
thread to generator random numbers, a thread for recording model outputs, and then
arbitrary number of threads processing iterations.

* Batch processing: Define a batch-size $B$, in which there are $B$ iterations
  that are processed on each thread.

  * Computationally more efficient as we saturate a threads more efficiently

This leads us to the requirement that there should be a convergence-controller
that should be in charge of dispatching these scenario runs.

# Scenario dispatcher

Mainly, the dispatcher should be able to

<!-- FIXME: -->
* Send signals to dispatched scenario-batches about changing values or ???

# Storing model outputs

Relay model outputs to a database saved on desk.
Use current commit-id of the code to ear-tag the simulation with a model
version.
Use also [semantic versioning](https://semver.org/) of the model in the model run.

Use also a type that contains all necessary information to run a scenario;
This necessitates that there are zero hardcoded default values built into the
the code itself, but those are retrievable from one common place, e.g. a
`ScenarioConfiguration` type.

# Regulartors

- [ ] Describe the hooks necessary to facilitate the regulators that may be
      used in the framework. Go through ASF and FMD regulators figure that out

There are implicit-regulators, but by and large they are 
# References

::: {#refs}
:::
