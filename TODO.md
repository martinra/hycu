TODO

- implement tests for merger

StoreDB
---

- read previous records in worker pool
- send to MPI workers (if MPI is used)
- initialize StoreDB record in each worker

- in master set a timer to store data
- if timer emits signal, ask via mpi for a current store data
  - how should we insert that in the block assingment cycle?
- gather it, accumulate it in the current store, and save using current time
- second save record using that same time
