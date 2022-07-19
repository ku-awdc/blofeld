//! Discrete Event Simulator System
//! 
//! A discrete event simulator should be composed of different modules,
//! each given a different weight to the event, and then these must be allotted
//! at some point.
//! 
//! 

type Weight = crate::parameters::Rate;

trait EventSimulator {
    type Individual;
    type Outcome;

    fn aggregate_and_sample() -> dyn Iterator<Item=(Self::Individual, Self::Outcome)>;

    fn register_module(event_module: impl EventModule<Self::Individual, Self::Outcome>);

}

trait EventModule<Individual, Outcome> {
    fn weights(weights: &[Individual]) -> dyn Iterator<Item=(Individual, Weight)>;
}   
