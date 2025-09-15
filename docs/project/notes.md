# Claude Code VIBE prompts

These are the user note

## Setup

- Create DOC/SPEC.md that sumerises and indexes all of the other md files in DOC/MVP/ and DOC/WORLD
- Create DOC/IMPLEMENTATION_PLAN.md that breaks down each phase and step in order to create the MVP. Make the phaeses and steps as small as possable for ease of implementaion and emphasise testeing, documentation and git management with tags.
- I have created a git repo at `git@github.com:MineForeman/veyrm.git`, create an appropriate gitignore and do the first check in.

## Step

- Detail what is needed to complete 0.1 Initialize Repository in its own document.
- Implement the items in 0.1 Initialize Repository document.

- Detail what is needed to complete 0.2 Dependencies Setup in its own document.
- Implement the items in 0.2 Dependencies Setup document.
- Check that was 0.2 Dependencies fully implementated.
- WHen I run the binary with ./build/bin/veyrm it charshes
- Update the documentation for 0.2 Dependencies Setup
- Tag and push 0.2 Dependencies Setup
- Update the IMPLEMENTATION_PLAN.md

- Detail what is needed to complete 0.3 in its own document.
- Implement the items in 0.3 document.
- Update the documentation and IMPLEMENTATION_PLAN.md and any other md files that need updating for 0.
- Tag and push 0.3, ensure that all files are checked in.

- Detail what is needed to complete 0.4 in its own document.
- Tag and checkin 1.1.

- Document what is necessary for 1.2 Turn System
- Implement 1.2 Turn System (with testing support)
- Add automated testing with --keys parameter
- Add frame dump mode for analysis
- Update implementation plan

- Document what is necessary for 1.3 Main Game Loop  
- Implement 1.3 Main Game Loop
- Update implementation plan

- Document what is necessary for 2.1 Tile System
- Implement 2.1 Tile System
- Update implementation plan and push

can you tell me why you use [[maybe_unused]], I have not seen that before and would like to know more
Test
Can you make it so that I dont have to escape characters when sending keys to build.sh

Can you make it so that build.sh reports on the binary size and other stats on the main menu.  Also, remove the Press Enter to continue... messages and just go back.

Create any testss needed and run them.

Document all changes sinse the last check in.  Update the existing md documents as well.  Add any additional md documents as nessery.

Do a git add -A, tag, check in and push.
Do a git add -A, check in and push.

Add an option to build.sh that creates a gorse video in a tmp directory so it does not check in.

I just played a game, the only log generated was veyrm_debug.log, why?

The "load" screen loks odd, jazz it up!

Implement doors and bind a key for players to use to toggle the door open and close them, also add it to the log.

Implement and log 8.3 Basic AI

I declear that this is the MVP!  Document and tag it as such.  Create a branch for it so we can preserve it for posterity and the switch back to the main branch.

Create a new brach where we will refactor and optimize and switch to it.

I want to refactor the documentation, search through the md files and document what could be improved.
I want to refactor abd optimize the classes and implement Doxygen comments, search through the md files and document what could be improved.  Refer to tmp/veyrm_classes.svg.

TODO

Seach through the code base and find TODO code and document it.

Redundent

Seach through the code base and find any redundent code and document it.

Tests

gcovr

Comments

Doxygen

MySQL

I want to transition to MySQL for all data I have fired up a docker container for the perpose:-

➜  verym git:(main) ✗ docker container list
CONTAINER ID   IMAGE                COMMAND                  CREATED      STATUS                PORTS                               NAMES
84d58a83f0bd   docker_mysql-mysql   "docker-entrypoint.s…"   4 days ago   Up 4 days (healthy)   0.0.0.0:3306->3306/tcp, 33060/tcp   mysql-local
➜  verym git:(main) ✗ ls ../docker_mysql
data               docker-compose.yml Dockerfile
➜  verym git:(main) ✗ cat ../docker_mysql/Dockerfile

## ./Dockerfile

FROM mysql:8.4

### (Optional) You can add custom configs later by COPY-ing a my.cnf into /etc/mysql/conf.d/

### Example

### COPY my.cnf /etc/mysql/conf.d/my.cnf

### RUN chmod 0444 /etc/mysql/conf.d/my.cnf

➜  verym git:(main) ✗ cat ../docker_mysql/docker-compose.yml

## ./docker-compose.yml

version: "3.9"

services:

  mysql:

    build: .
    container_name: mysql-local
    restart: unless-stopped
    environment:
      # Change these to something secure before first run
      MYSQL_ROOT_PASSWORD: change-me
      MYSQL_DATABASE: appdb
      MYSQL_USER: appuser
      MYSQL_PASSWORD: change-me-too
      TZ: Pacific/Auckland
    ports:
      - "3306:3306"
    # On macOS, bind mounts are fine; this keeps MySQL data in ./data on your host
    volumes:
      - ./data:/var/lib/mysql:delegated
      # If you later want to auto-run SQL on first startup:
      # - ./initdb:/docker-entrypoint-initdb.d:ro
    command:
      - --character-set-server=utf8mb4
      - --collation-server=utf8mb4_unicode_ci
      # Uncomment if you need classic auth for older clients:
      # - --default-authentication-plugin=mysql_native_password
    healthcheck:
      test: ["CMD", "mysqladmin", "ping", "-h", "127.0.0.1", "-uroot", "-pchange-me"]
      interval: 10s
      timeout: 5s
      retries: 10

I want everythiong in data/, logs/ and saves/ to be moved there.

Tables I want;-

player (for indiviual players)
charachter (for players characters)
character_state (for the current character state)
character_inventory (for the characters inventory)
monsters ( for the monsters)
items (for items)
and anything else needed.

I want build.sh to be able to load, clear, view, backup and reload the data.

Document was must be done.

Multiplayer

- Split the server and clinent into separate binaries.
- Start the server an then start the client.
- All traffic between the server and client must be done over the network, later they wont even be on the same machiene.
- Must be server authritive.
- As much as the "heavy lifting" as possable be done by the client.

Ranged attacks
Spell system
Classes
Races
Character Generation
Character Progression
Town
Quests
Help
Auction House
World Map
Crafting
Player Houseing
Guilds
Guild Halls
