absolutely minimalist key-value storage

v0.1 uses a text file for storage and supports naive `put`, `get`, and `show`
v0.2 refactored, added a test bench; 10k operations take 71 seconds on average on my machine
v0.3 using hashmap and primitive WAL for persistence; 10k operation take less than 1 second