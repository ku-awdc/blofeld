//! BLOFELD is a modelling framework for infectious diseases.
//! 
//! 
mod population {
    //! A population trait is defined here, 
}

mod regulators {
    //! Also known as `authorities`
    //! 
    //! 
    //! Examples of regulators would be
    //! 
    //! * 
}

mod disease {
    
}

mod default_state {

    /// Every simulation module must have a 
    pub trait DefaultNewState { 
        /// Returns that contains itself 
        fn empty() -> Self;
        fn empty_with_capacity(capacity: usize) -> Self;
        /// Once an iteration is completed, this is called to (cheaply) 
        /// reset the module.
        fn reset_state(&mut self) -> Self;
    }
}

mod advance_state { 

    /// Every simulation module must have a unified update/advance state method
    pub trait DynamicProcess {
        fn update(&mut self); 
    }
}
