# Multifork

A long time ago in a galaxy far away, I read about a project named Telefork.
Telefork was a tool that allowed for running processes to be transferred over the network from one host to another.
This was accomplished by temporarily pausing, copying all process memory, and sending it off to a forkserver service that would recreate the process before letting it run free.

To date, I have not been able to locate this mythological project I read about in a fever dream.
If anyone knows the location of the original project, please file an issue against this repo and let me know.

Ultimately, the goal is answer the following questions:
- How can a multi-threaded process be forked while maintaining it's multi-threaded state?
- How can a multi-process program be "forked"?
- How can a running program (generally) be transferred to a new host and immediately resumed?

These are **goals**, and certainly not reflective of the current state of this repository.
