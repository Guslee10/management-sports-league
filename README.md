# 🏆 Management of the Sports League

Sports league management system developed in **C** with a **PostgreSQL** database. 
It allows for the administration of players, teams, matches, and statistics for a university league.

## ✨ Features

1. **Sign player** - Registers a new player
2. **Individual ranking** - Displays players by average
3. **Update match** - Adds points to players
4. **Team ranking** - Team classification
5. **Remove player** - Removes a player
6. **Search player** - Search by name
7. **Transfer** - Moves a player to another team
8. **Register match** - Loads match result
9. **Standings table** - Tournament positions
10. **Simulate match** - Generates a random match
11. **Team roster** - Complete team squad
12. **Export list** - Saves to a .txt file

## 🛠️ Requirements

- GCC (C compiler)
- PostgreSQL 13 or higher
- libpq-dev library

## ⚙️ Setup

1. Create database:
```sql
CREATE DATABASE liga_univ_bd;

2. Execute schema:

psql -U postgres -d liga_univ_bd -f database/schema.sql

## 🚀 Compilation:
```bash
make
./liga
```

## 📁 Structure:
```
Management of the sports league/
├── main.c
├── Makefile
├── .gitignore
├── README.md
└── database/
    └── schema.sql
```
```