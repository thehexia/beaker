
// ------------------------------------------------ //
//      Instructions


struct Instruction { };

// Write an output to port acttion
// to the context
struct Write_output : Instruction
{

};




// ------------------------------------------------ //
//      Required Actions

struct Action { };

struct Apply_set_field : Action
{

};


struct Apply_add_field : Action
{

};


struct Apply_rmv_field : Action
{

};


// Output the packet to a given
// port.
struct Apply_output : Action
{

};


// Goto a group table
struct Apply_group : Action
{

};
