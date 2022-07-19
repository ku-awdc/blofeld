---
title: Model Requirement Sheets
subtitle: **Version 4 -- 2020-12-09**
date: 2020-12-09
---
# **Model Requirement Sheets** **for BT, FMD, ASF and AI**

## **Overview**

The requirements given below are broken down into the following
categories:

- *User interface* refers to the way in which users interact with the
    model

- *Technical* refers to aspects of the way in which the models are
    coded that are entirely technical in nature (e.g. driven by
    computational performance)

- *Data* refers to observations that are either used directly by the
    models (e.g. farm locations) or indirectly to inform parameter
    choices or network structures within the model (e.g. weather data
    informing wind patterns)

- *Risk assessment model* refers to a risk-based model of disease
    introduction into Denmark from external sources (only relevant for
    avian influenza)

- *Disease model* refers to the model of disease transmission between
    animals within a unit of animals (e.g. a farm or discrete area of
    wildlife habitat), where a typical assumption would be random mixing
    within these animals

- *Spread model* refers to the model of disease spread between units
    of animals, where a typical assumption would be structured contacts
    between units (e.g. distance-driven)

- *Detection layer* refers to the aspects of the model relating to the
    observation/detection of disease by farmers, veterinarians or
    authorities

- *Intervention layer* refers to the aspects of the model relating to
    the control of disease by farmers, veterinarians or authorities in
    response to detection -- this could be implemented as a separate
    aspect of the model or together with detection as a single
    "surveillance" layer

- *Macro layer* refers to higher-level outputs of the model relating
    to e.g. economic impact or resource utilisation associated with a
    simulated outbreak

Within each category a list is given under the following subheadings:

- *Essential* means that the models must contain these features in
    order to fulfil our contractual obligations with FVST and/or
    requirements for ongoing projects

- *Desirable* means that the features would either allow more
    advanced/realistic modelling options or allow us to respond more
    quickly/efficiently to additional/unexpected requests from FVST in
    the event of an outbreak

*Note: In order to avoid unnecessary duplication, the majority of the
requirements are given under common lists for (a) all four diseases and
(b) FMD, BT and ASF (excluding AI), with disease-specific additions
specified as shorter following sections.*

*Note: This requirement sheet should be interpreted as a "wish-list" and
does not necessarily reflect what is actually included in the models we
have available -- identifying the requirements that are currently
missing will be a separate exercise.*

**General Requirements for all Diseases**

**User interface**

Essential:

- Code is self-contained and runnable by a minimum of one person in
    AWDC \*

- Inputs are documented

- Inputs are updateable by a minimum of one person in AWDC \*

- All possible outputs are well described

- At least one person in AWDC \* is able to understand, describe and
    analyse outputs at all times where models are expected to be
    available

Desirable:

- Usage fully documented and examples/vignettes provided

- Interface uses consistent code across diseases

- Input formats standardised across diseases

- Output formats standardised across diseases

- More than one person at AWDC \* is able to run models, update
    inputs, and analyse outputs

*\* Where models are indicated to be usable/runnable by "one person in
AWDC" above, this should be interpreted as "one person in AWDC at all
times where models are expected to be available" to allow for the
possibility of models not being available during periods where the key
individual is travelling or on holidays. We also emphasise that the need
for redundant capacity in the event of unexpected illness and/or
resignation of staff implies that at least one additional AWDC employee
should be able to take over at relatively short notice.*

## **Technical/Coding**

Essential:

- Code is internally validated and guaranteed to produce reproducible
    results

Desirable:

- Code is fully contained within a dedicated package (one package per
    disease)

- Code is contained within a central repository with versioning

- Code is externally validated against either real outbreak data or
    results from an independent model

- Validation exercises should be repeated when changes are made to the
    model

  - Via output testing, unit testing and/or continuous integration
        approaches

- Models should run as efficiently as possible to facilitate
    replication of outputs

  - Use of entity component systems may be an advantage

  - Automated daily runs for risk assessment (in particular for AI)

- Spread models should use networks to provide full transparency and
    flexibility, with options for use of simple static networks where
    these suffice and more complex temporal/dynamic networks where
    needed to describe e.g. seasonality**\
    **

## **General Requirements for FMD, ASF and BT**

### **Data**

*\[Note: requirements for pigs do not apply to BT, and cattle/goat/sheep
do not apply to ASF\]*

Essential:

- Recent Danish data on domestic pigs, cattle, goat and sheep numbers,
    including spatial location, herd size and herd type (including
    markets)

- Recent Danish movement data for domestic pigs, cattle, goats and
    sheep

- Distribution of herd numbers per route for milk trucks and Daka for
    domestic pigs, cattle, goats and sheep

- Economic data

- Capacity data (Daka collections and rendering plants, vaccine, FVST,
    test/lab, equipment)

Desirable:

- Distributions of indirect contacts (between farmers, inseminators,
    veterinarians and other consultants, milk controllers, visitors,
    feedstuff)

- Behaviour/cultural-based data

  - Efficiency of movement restrictions

  - Efficiency of detection (e.g. variability of disease awareness
        between farms)

  - Efficiency of intervention (e.g. variability of biosecurity
        compliance)

```{=html}
<!-- -->
```

- Weather data (temperature, wind patterns, humidity/rainfall and
    sunshine) based on spatial location and time of year

  - Note: potential use of the RimPuff model from DMI

- Near real-time (i.e. data with \<7 days' delay) updates of data on
    domestic pig, cattle, goat and sheep numbers & movements

- Contacts between farm and external people due to equipment sharing

### **Disease model (within farm/habitat unit)**

Essential:

- Must use farm-level numbers of animals informed by data

- Must allow disease to spread between animals within that unit,
    taking account of disease transmission between animals of the same
    species and different species as relevant to the disease and unit

- Must allow external interventions to reduce on-farm prevalence of
    disease

- Must allow disease to be "seeded" within the unit via the user
    interface

- Use of parameters must be documented as to the origin of the
    parameter values/assumptions used (i.e. source of data and/or
    literature reference)

- Must allow heterogeneity between farms in terms of efficacy of
    detection and intervention based on whatever data/parameters are
    available

Desirable:

- Parameter values should be amendable as inputs rather than being
    hard coded

- Spread in relevant wildlife species (deer, wild boar) should be
    modelled specifically

## **Spread model (between farm/wildlife units)**

Essential:

- Unit-to-unit spread based on animal movements and other
    anthropogenic spread

  - Including the influence of markets, shows and dealers

- Farms must use either an actual spatial location or some data-driven
    proxy

- Between-unit spread must be within and between species as relevant
    for the disease

- Must use predominant wind patterns to inform a spatial kernel
    (except for ASF)

- Must allow indirect contacts (people and materials as listed under
    "Data")

Desirable:

- Near real-time (\<7 days' delay) inclusion of animal movement data

## **Detection layer**

Essential:

- Probability of detection due to passive surveillance

  - Before outbreak detection

  - After outbreak detection

- Probability of detection due to active surveillance

  - In ring zones

  - Due to tracing

- Resources

  - Test capacity (laboratory capacity)

  - Visitation capacity (FVST manpower)

Desirable:

- Code modularity that facilitates the flexibility to quickly add new
    detection options in "war time" without having unintended side
    effects within the code

## **Intervention layer**

Essential:

- EU-specified control measures including:

  - Movement controls

    - Movement bans based on zones

    - Movement bans based on tracing

  - Mandated depopulation of specific farms

    - E.g. within a defined distance of known case farms

- Resources

  - Prioritisation of interventions (i.e. farm ranking) based on
        available resources

  - Culling capacity

    - FVST capacity

    - Daka capacity

  - Vaccination capacity and efficiency

    - Vaccine availability

    - Manpower

- National/regional standstill

- Increased detection efforts (active surveillance)

  - Based on ring zones

  - Also based on tracing

- Vaccination

  - Vaccination to live (BT only)

  - Vaccination to kill (FMD and ASF)

Desirable:

- Code modularity that facilitates the flexibility to quickly add new
    intervention options in "war time" without having unintended side
    effects within the code

- Prioritisation of testing (i.e. which farms first) based on
    available resources

- Possibility for repopulation accounting for cleaning and
    disinfection of farms

## **Macro-level outputs**

Essential:

- Economic analysis of the industry costs of an outbreak

  - Direct

  - Indirect

  - State costs vs industry costs (direct costs)

- Resource utilisation assessment/output

  - On a different time-scale to other outputs (i.e. daily basis)

  - Manpower for different tasks

  - Total use of equipment

**\
**

### **Specific Requirements for FMD**

**Data**

Essential:

- Distribution of indirect contacts (between farmers, inseminators,
    veterinarians and other consultants, milk controllers, visitors,
    feedstuff)

- Behaviour/cultural-based data

  - Efficiency of movement restrictions

  - Efficiency of detection (e.g. variability of disease awareness
        between farms)

  - Efficiency of intervention (e.g. variability of biosecurity
        compliance)

*\[Note: these requirements are upgraded from "Desirable" in the general
section\]*

Desirable:

- Near real-time (\<7 days' delay) wind pattern data at farm or
    regional level

  - Note: potential use of the RimPuff model from DMI

- Potentially wildlife locations (deer and wild boar) although low
    priority

**Spread model (between farm/wildlife units)**

Essential:

- Heterogeneity in spread between farms to account for heterogeneity
    in terms of behaviour/cultural data between farms

Desirable:

- Near real-time (\<7 days' delay) inclusion of wind pattern data at
    regional level

- Wildlife locations based on real data

**\
**

**Specific Requirements for BT**

**Data**

Essential:

- Average temperature data based on spatial location and time of year

- Wind pattern data based on spatial location and time of year

  - Note: potential use of the RimPuff model from DMI

Desirable:

- Near real-time (\<7 days' delay) wind pattern data at farm or
    regional level

  - Note: potential use of the RimPuff model from DMI

- Habitat data at farm level (proxy for midge burden)

**Disease model (within farm/wildlife unit)**

Essential:

- Explicit model of vector multiplication and disease transmission

  - Must account for temperature

- Explicit host-vector interaction model

Desirable:

- Near real-time (\<7 days' delay) inclusion of temperature data at
    farm level

- Heterogeneity in midge numbers between farms informed by data

**Spread model (between farm/wildlife units)**

Essential:

- Seasonal risk of disease transmission due to varying midge survival

Desirable:

- Near real-time (\<7 days delay) inclusion of wind pattern data at
    farm/regional level

**\
**

**Specific Requirements for ASF**

**Data**

Essential:

- Distribution of baseline mortality figures across domestic pig farms

- Wild boar habitat data

- Data on wild boar observations, reproduction, movements etc.

- Data on distribution of number of farms visited for trucks moving
    domestic pigs to slaughter

Desirable:

- Specific mortality data from individual farms

**Disease model (within farm/wildlife unit)**

Essential:

- Probability of transmission from WB to domestic pigs

- Survival time of virus relating to indirect contacts

  - Seasonal (temperature)

- Survival time of virus in carcasses for contact between wild boar

  - Seasonal (temperature)

- Time from infection to clinical signs/detection

- Case-fatality rate

Desirable:

- Survival time on vectors informed by temperature

**Detection layer**

Essential:

- Passive surveillance in wild boar (testing of carcasses)

**Intervention layer**

Essential:

- Options to control wild boar:

  - Fences

  - Shooting

  - Based on zones

- Culling options for domestic pigs:

  - Detected

  - Zones

  - Traced

  - Herd types

Desirable:

- Efficacy of potential wild boar vaccination

**\
**

**Requirements for Avian Influenza**

**Data**

Essential:

- Real-time API or near real-time transfer of surveillance data from
    ai.fvst.dk

- Real-time API or near real-time data from outside Denmark (OIE:
    <https://www.oie.int/en/animal-health-in-the-world/update-on-avian-influenza/2020/>
    or WAHIS and other relevant sources)

- Recent data on locations, species, sizes and types (indoor/outdoor)
    of Danish poultry farms

Desirable:

- Data on game bird releases

- Real-time data (\<7 days) on bird observations (possibly from DOF)

**Risk assessment model**

Essential:

- Risk of introductions based on exogenous risk (based on OIE data)

  - Multiple introductions per year possible

**Disease model (within-flock/wildlife unit)**

Essential:

- Must use known (estimated) numbers of wild birds (temporally
    changing with species), game birds, duck farms and poultry farms

Desirable:

- Numbers, time and locations of released game birds

**Spread model (between flock/wildlife units)**

Essential:

- Potential spread of AI between locations of:

  - Wild birds

  - Poultry farms

  - Duck farms

- Estimate of risk from incoming migratory birds

- Updated data on control measures implemented

**Detection layer**

Essential:

- Probability of detection due to passive / active surveillance

  - Before outbreak detection

  - After outbreak detection

- Dead wildlife passive reporting

- Active screening of wild birds (hunters etc) and farms

Desirable:

- Suggestions for targeted surveillance towards time, locations and
    species

**Intervention layer**

Essential:

- EU-specified control measures including:

  - Roofing of poultry farms

  - Mandatory culling of positive farms

Desirable:

- Prioritisation of interventions based on available resources

- Ability to quickly add new intervention options in "war time"

**Macro-level outputs**

Desirable:

- Economic analysis of the industry cost of an outbreak

  - Direct

  - Indirect

  - State costs vs industry costs

- Resource utilisation assessment/output

  - On a different time-scale to other outputs (i.e. daily basis)

  - Manpower for different tasks

  - Total use of equipment
