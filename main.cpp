#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;


const int MAX_INV = 10;

void typeText(const string& text, int delay_ms = 50) {
    for (char c : text) {
        cout << c << flush;
//        Sleep(delay_ms); // Windows Sleep in milliseconds
    }
    cout << endl;
}
// Player Model

struct Player {
    string name;
    int health;
    int location;
    string inventory[MAX_INV];
    int inv_count;
    bool found_clue;
    bool angered_spirit;
    bool visited_ayan;
};


bool saveGame(const Player &p, const string &filename) {
    ofstream fout(filename);
    if (!fout) return false;

    fout << "NAME:" << p.name << "\n";
    fout << "HEALTH:" << p.health << "\n";
    fout << "LOCATION:" << p.location << "\n";
    fout << "INV_COUNT:" << p.inv_count << "\n";
    for (int i = 0; i < p.inv_count; i++)
        fout << p.inventory[i] << "\n";
    fout << "FOUND_CLUE:" << (p.found_clue ? 1 : 0) << "\n";
    fout << "ANGERED_SPIRIT:" << (p.angered_spirit ? 1 : 0) << "\n";
    fout << "VISITED_AYAN:" << (p.visited_ayan ? 1 : 0) << "\n";

    fout.close();
    return fout.good();
}

bool loadGame(Player &p, const string &filename) {
    ifstream fin(filename);
    if (!fin) return false;

    string line;
    p.inv_count = 0;
    while (getline(fin, line)) {
        if (line.rfind("NAME:", 0) == 0)
            p.name = line.substr(5);
        else if (line.rfind("HEALTH:", 0) == 0)
            p.health = stoi(line.substr(7));
        else if (line.rfind("LOCATION:", 0) == 0)
            p.location = stoi(line.substr(9));
        else if (line.rfind("INV_COUNT:", 0) == 0) {
            // Ignores code here, items are read later
        } else if (line.rfind("FOUND_CLUE:", 0) == 0)
            p.found_clue = stoi(line.substr(11));

        else if (line.rfind("ANGERED_SPIRIT:", 0) == 0)
            p.angered_spirit = stoi(line.substr(15));

        else if (line.rfind("VISITED_AYAN:", 0) == 0)
            p.visited_ayan = stoi(line.substr(13));
        else {
            if (p.inv_count < MAX_INV)
                p.inventory[p.inv_count++] = line;
        }

    }

    fin.close();
    if (p.health < 0)
        p.health = 0;
    if (p.health > 100)
        p.health = 100;

    return true;

}

// Scene prototypes
int scene_start(Player &p);
int scene_forest(Player &p);
int scene_cabin(Player &p);
int scene_road(Player &p);
int scene_city(Player &p);
int scene_ayanHouse(Player &p);
int scene_mall(Player &p);
int scene_finalArea(Player &p);
void scene_secretEnding();

void displayStatus(const Player &p);

// Helper functions

void initPlayer(Player &p) {
    p.name = "Player";
    p.health = 100;
    p.location = 0;
    p.inv_count = 0;
    p.found_clue = false;
    p.angered_spirit = false;
    p.visited_ayan = false;
}

bool addItem(Player &p, const string& item) {
    if (p.inv_count >= MAX_INV) return false;
    p.inventory[p.inv_count++] = item;
    return true;
}

bool removeItem(Player &p, const string& item) {
    for (int i = 0; i < p.inv_count; i++) {
        if (p.inventory[i] == item) {
            // shift left
            for (int j = i; j < p.inv_count - 1; j++)
                p.inventory[j] = p.inventory[j + 1];
            p.inv_count--;
            return true;
        }
    }

    return false;
}


bool hasItem(const Player &p, const string& item) {
    for (int i = 0; i < p.inv_count; i++)
        if (p.inventory[i] == item)
            return true;
    return false;
}

// Reads choices for safe input during scenes

int readChoice(int minVal, int maxVal) {
    int choice;
    while (!(cin >> choice) || (choice < minVal || choice > maxVal)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid. Choose between " << minVal << " and " << maxVal << ": ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline
    return choice;
}

int startCombat(Player &p, int enemyHP, const string &enemyName) {
    cout << "\n*** Combat: " << enemyName << " appears! ***\n";

    // Quick health check
    if (p.health <= 0) return -1;

    while (p.health > 0 && enemyHP > 0) {
        cout << "\nYour HP: " << p.health << "  " << enemyName << " HP: " << enemyHP << "\n";
        cout << "1) Attack\n2) Use Medkit (if you have one)\n";
        cout << "Choice: ";
        int choice = readChoice(1, 2);

        if (choice == 1) {
            int dmg = (rand() % 11) + 10; // 10 - 20
            enemyHP -= dmg;
            cout << "You strike the " << enemyName << " for " << dmg << " damage.\n";
        } else if (choice == 2) {
            if (hasItem(p, "Medkit")) {
                removeItem(p, "Medkit");
                p.health += 25;
                if (p.health > 100) p.health = 100; // Makes sure HP doesn't go over max of 100
                cout << "You used a Medkit and restored 25 HP.\n";
            } else {
                cout << "You don't have a Medkit!\n";
            }
        }

        //Enemy's turn (if still alive)
        if (enemyHP > 0) {
            int enemyDmg = (rand() % 11) + 5; // 5 - 15
            p.health -= enemyDmg;
            cout << enemyName << " hits you for " << enemyDmg << " damage!\n";
        }
    }

    if (p.health <= 0) {
        cout << "\nYou have succumbed to your wounds...\n";
        return -1; // Player died
    }
    cout << "\nYou defeated the " << enemyName << "!\n";
    return 0; // Victory
}


int scene_start(Player &p) {
    typeText("\nWill you choose to enter the portal?", 30);
    cout << "1) Enter the portal\n2) Turn back and run\nChoice: ";

    int choice;

    choice = readChoice(1, 2);

    if (choice == 1) return 1; // scene_forest
    else return 2; // scene_secretEnding

}

void scene_secretEnding() {
    typeText("\n*** YOU CHOOSE TO RUN AWAY. YEARS PASS BY AND AYAN IS NEVER FOUND. YOU ARE CONSUMED WITH THE GUILT OF NEVER TRYING TO FIND HIM. SECRET ENDING ***", 40);
}

int scene_forest(Player &p) {
    typeText("\nEmerging from the portal, you find yourself in a forest. You search the forest carefully. The ground is disturbed, branches are broken,", 30);
    typeText("and strange footprints lead deeper into trees. The silence is unsettling but you continue, hoping to find something useful.\n", 30);
    typeText("You stumble on an abandoned cabin. Will you enter?", 40);
    cout << "1) Enter\n2) Keep going and ignore the cabin\nChoice: ";
    int choice;
    choice = readChoice(1,2);

    if (choice == 1)
        return 3; // scene_cabin
    else
        return 4; // scene_road

    }

int scene_cabin(Player &p) {
    typeText("You enter the crumbling cabin and find a medkit!. You continue to move on.\n", 30);
    bool added = addItem(p, "Medkit");

    return 4; // scene_road
}

int scene_road(Player &p) {
    typeText("\nYou keep moving on the road. All of a sudden, you hear a horrifying shriek. You turn around to find a horrifying monster with many teeth.", 30);
    typeText("It hasn't noticed you yet", 30);
    typeText("What do you choose?", 30);
    cout << "1) Fight\n";
    cout << "2) Try sneaking away\n";
    cout << "Choice: ";
    int choice = readChoice(1, 2);
    int result;
    if (choice == 1) {
        result = startCombat(p, 50, "Monster");
        if (result == -1) {
            return -1; // death
        } else if (result == 0) {
            // victory
            return 5; // scene_city
        }
    } else {
        // Sneaking away (20% chance of success)
        int roll = rand() % 100; // random number between 0-99

        if (roll < 20) {
            typeText("\nYou quietly step back... The monster doesn't notice you. You successfully avoid the confrontation.", 30);

            return 5; // scene_city
        } else {
            typeText("\nYou step on a branch-CRACK! The monster turns toward you and attacks!", 30);
            typeText("You start combat with reduced HP.", 30);
            p.health -= 50;
            result = startCombat(p, 50, "Monster");

            if (result == -1) return -1; // death
            return 5; // scene_city
        }

    }

}

int scene_city(Player &p) {
    typeText("\nYou have now entered the city.\n", 30);
    typeText("The sky above the ruined city is a sickly red,\n", 30);
    typeText("and the air smells like burned metal. Buildings stand twisted, melted, or half-collapse.\n", 30);
    typeText("This was once your home... but now it feels hostile.\n\n", 30);

    typeText("As you step onto the cracked streets, you notice something unexpected:\n", 30);
    typeText("- A faint trail of footprints, small, human... maybe Ayan's.\n", 30);
    typeText("- And in the opposite direction: drag marks and dried black blood.\n\n", 30);

    // If player has NOT visited the house,  give both options
    if (!p.visited_ayan) {
        typeText("Where will you go?\n", 30);
        cout << "1) Follow the footprints toward Ayan's house\n";
        cout << "2) Investigate the blood trail leading to a destroyed mall\n";
        cout << "Choice: ";
        int choice = readChoice(1, 3);
        if (choice == 1) {
            typeText("\nYou follow the faint footprints deeper into the ghostly streets...\n", 30);
            return 6; // scene_ayanHouse
        }
        else {
            p.angered_spirit = true;
            typeText("\nYou follow the blood trail toward the ruins of the old mall...\n", 30);
            return 7; // scene_mall
        }
    }
    // If player already visited the house, remove that option: only mall
    else {
        typeText("You've already searched Ayan's house. You will now head to the mall\n", 30);
        p.angered_spirit = true;
        return 7; // scene_mall
    }

}

int scene_ayanHouse(Player &p) {
    p.visited_ayan = true;
    typeText("\nYou arrive at Ayan's house. The front door hangs crooked, the porch is ash-covered.\n", 30);
    typeText("You can feel a strange cold coming from inside.\n\n", 30);

    cout << "Choices:\n";
    cout << "1) Enter quietly and search the living room\n";
    cout << "2) Break in and search the basement (risky)\n";
    cout << "3) Check the backyard for clues\n";
    cout << "Choice: ";

    int choice = readChoice(1, 3);

    if (choice == 1) {
        typeText("\nYou find Ayan's phone under a sofa cushion. A short voice note plays : he mentions the 'old factory' and a ritual.\n", 30);
        p.found_clue = true;
        addItem(p, "AyanPhone");
        typeText("Found: AyanPhone (clue). This might help in the final area.\n", 30);
        typeText("You leave the house and head to the old factory as mentioned in the note.\n", 30);
        return 8; // scene_finalArea
    } else if (choice == 2) {
        typeText("\nYou smash the basement door open. The noise attracts a monster! You are ambushed\n", 30);
        int result = startCombat(p, 60, "Basement Fiend");
        if (result == -1) return -1; // died
        // victory or escape
        typeText("In the basement you find a torn note mentioning a ritual at the 'old factory' and a weak spot: 'the lights'. You also find a Medkit!\n", 30);
        typeText("You leave the house and head to old the factory as mentioned in the note.\n", 30);
        p.found_clue = true;
        addItem(p, "TornNote");
        addItem(p, "Medkit");
        return 8; // scene_finalArea
    } else {
        // backyard: low chance to find a helpful item
        int roll = rand() % 100; // Random number between 0 and 99
        if (roll < 40) {
            typeText("\nYou find a small charm that calms the spirits (+10 HP)\n", 30);
            p.health = min(100, p.health + 10);
        } else {
            typeText("\nYou find only scorched soil. Nothing useful.\n", 30);
        }
        return 5; // Back to city to choose again
    }
}

int scene_mall(Player &p) {
    typeText("\nYou approach the destroyed mall. The roof has caved in and the inside is pitch black.\n", 30);
    typeText("A cold wind blows from within... almost like something breathing.\n", 30);

    typeText("\nWhere do you go?\n", 30);
    cout << "1) Enter the mall through the main entrance\n";
    cout << "2) Circle around and check the loading docks\n";
    cout << "3) Search the parking lot\n";
    cout << "Choice: ";
    int choice = readChoice(1, 3);

    if (choice == 1) {
        typeText("\nYou step inside... The darkness swallows you.\n", 30);

        if (p.angered_spirit) {
            typeText("A deafening scream erupts inside your head-there's a furious spirit in the mall!\n", 30);
            typeText("You are attacked!\n", 20);
            int result = startCombat(p, 70, "Vengeful Spirit");
            if (result == -1) return -1;

            typeText("\nAfter defeating the spirit, you see glowing footprints leading deeper in.\n", 30);
        } else {
            typeText("A whisper echoes: 'You should not have come here...'\n", 40);
        }

        typeText("You find a staircase leading downward-toward the old factory district.\n", 30);
        return 8; // scene_finalArea
    } else if (choice == 2) {
        typeText("\nAt the loading docks, you find shattered crates and dried blood.\n", 30);
        typeText("You also find a Medkit.\n", 30);
        addItem(p, "Medkit");

        typeText("A trail of black sludge leads toward the factory district.\n", 30);
        return 8; // scene_finalArea
    } else {
        typeText("\nYou search between the rusted cars.\n", 30);
        int roll = rand() % 100;

        if (roll < 30) {
            typeText("You find an old crowbar. You take it (+10 damage in next fight).\n", 30);
            addItem(p, "Crowbar");
        } else {
            typeText("Nothing but dust and broken glass.\n", 30);
        }

        typeText("You notice a large hole in the mall wall that exits toward the factory.\n", 30);
        return 8; // scene_finalArea
    }
}

int scene_finalArea(Player &p) {
    typeText("\nYou arrive at the abandoned factory, the heart of this nightmare.\n", 30);
    typeText("The sky rumbles. The ground shakes. Something is waiting inside.\n\n", 30);

    typeText("As you enter, you see Ayan suspended by dark tendrils of shadow.\n", 30);
    typeText("A monstrous entity turns toward you: The Ritual Guardian.\n\n", 30);

    bool prepared = p.found_clue;
    bool angry = p.angered_spirit;

    if (!prepared) {
        typeText("You don't understand the ritual. You don't know it's weakness.\n", 30);
        typeText("You must fight blindly.\n", 30);
    } else {
        typeText("Thanks to the clues you found, you know the creature is weak to light.\n", 30);
        typeText("There is a control panel that can activate floodlights.\n", 30);
    }

    typeText("\nWhat do you do?\n", 20);
    cout << "1) Fight the Ritual Guardian\n";
    if (prepared) cout << "2) Sprint to the control panel and activate the lights\n";
    else cout << "2) Try something desperate (low success)\n";
    cout << "Choice: ";
    int choice = readChoice(1, 2);

    if (choice == 1) {
        int result = startCombat(p, angry ? 120 : 90, "Ritual Guardian");

        if (result == -1) return -1; // death -> bad ending

        typeText("With the beast defeated, the tendrils binding Ayan collapse.\n", 30);
        typeText("You carry him out as the factory collapses.\n", 30);
        return 99; // good ending (combat victory)
    }

    // Option 2 logic
    if (prepared) {
        typeText("\nYou rush to the control panel and smash the switch.\n", 30);
        typeText("BLINDING WHITE LIGHT fills the factory!\n", 30);

        if (angry) {
            typeText("But the spirit you angered screams and slams into you!\n", 30);
            p.health -= 40;
            if (p.health <= 0) return -1; // death -> bad ending
        }

        typeText("The Ritual Guardian melts into ash.\n", 30);
        typeText("Ayan collapses into your arms.\n", 30);
        return 99; // good ending (smart victory)
    }

    // Unprepared attempt -> 20% success
    int roll = rand() % 100;
    if (roll < 20) {
        typeText("\nPure luck! You flip the right switch!\n", 30);
        typeText("Light erupts, vaporizing the monster.\n", 30);
        return 99; // good ending (risky victory)
    } else {
        typeText("\nYou pull a random lever-steam blasts into your face! (30 hp lost)\n", 30);
        p.health -= 30;
        if (p.health <= 0) return -1; // death -> bad ending
        typeText("The monster crushes you.\n", 40);
        return -1; // death -> bad ending
    }
}

int mainMenu() {
    cout << "\033[32m";
    typeText(
"=====================================================\n"
"   V V   OOOOO  IIIII  DDDD        OOOOO  FFFFF\n"
"   V V   O   O    I    D   D       O   O  F\n"
"   V V   O   O    I    D   D       O   O  FFFF\n"
"    V    O   O    I    D   D       O   O  F\n"
"    V    OOOOO  IIIII  DDDD        OOOOO  F\n"
"\n"
"    OOOOO  RRRR   EEEEE  GGGGG  OOOOO  N   N\n"
"    O   O  R   R  E      G      O   O  NN  N\n"
"    O   O  RRRR   EEEE   G GGG  O   O  N N N\n"
"    O   O  R  R   E      G   G  O   O  N  NN\n"
"    OOOOO  R   R  EEEEE  GGGGG  OOOOO  N   N\n"
"=====================================================\n", 1);
    cout << "\nWhich Option?\n\n";
    cout << " 1) Start New Game\n";
    cout << " 2) Load Existing Game\n";
    cout << " 3) Exit\n";
    cout << "Choice: ";
    cout << "\033[0m";

    return readChoice(1, 3);
}

void trySavePrompt(const Player &p) {
    cout << "\nWould you like to save your game? (1 = Yes, 2 = No): ";
    int choice = readChoice(1, 2);
    if (choice == 1) {
        if (saveGame(p, "save.txt"))
            cout << "Game saved to save.txt.\n";
        else
            cout << "Save failed (could not open file).\n";
    } else {
        cout << "Save skipped.\n";
    }
}

int main() {

    srand(time(0)); // Seed for random function
    Player mufasser;
    bool running = true;

    int menuChoice = mainMenu();
    cout << "\033[91m";

    if (menuChoice == 1)
        initPlayer(mufasser);
    else if (menuChoice == 2) {
        // Load game
        if (!loadGame(mufasser, "save.txt")) {
            cout << "Could not load save file! Starting new game instead.\n";
            initPlayer(mufasser);
        } else {
            cout << "Game loaded successfully!\n";
        }
    }
    else {
        cout << "Goodbye!\n";
        return 0;
    }



    if (mufasser.location == 0 && running) {
         typeText("\nYou live in the quiet town of Oregon. Everything was normal... until today.", 35);
typeText("Your friend Ayan suddenly disappeared near the old forest.", 35);
typeText("People say they heard strange noises, and some even saw lights coming from the woods.", 35);
typeText("As you follow the trail, you discover signs of a hidden world.", 35);
typeText("A portal that is completely void.\n", 35);

     // Get player name (use getline to allow spaces)
    cout << "Enter your name: ";
    getline(cin, mufasser.name);
    }

    while (running) {
        if (mufasser.location != 2 && mufasser.location != 99 && mufasser.location != -1)
            displayStatus(mufasser);
        if (mufasser.location == 3 || mufasser.location == 5 || mufasser.location == 6) { // Saves at major locations
            trySavePrompt(mufasser);
}

        if (mufasser.health <= 0) {
            cout << "\nYou strength fades and the world around you darkens. The creature's roar echoes as everything goes silent. Your journey ends here. \n";
            break;
        }

        switch (mufasser.location) {
            case 0:
                mufasser.location = scene_start(mufasser);
                break;
            case 1:
                mufasser.location = scene_forest(mufasser);
                break;
            case 2:
                scene_secretEnding();
                running = false;
                break;
            case 3:
                mufasser.location = scene_cabin(mufasser);
                break;
            case 4:
                mufasser.location = scene_road(mufasser);
                break;
            case 5:
                mufasser.location = scene_city(mufasser);
                break;
            case 6:
                mufasser.location = scene_ayanHouse(mufasser);
                break;
            case 7:
                mufasser.location = scene_mall(mufasser);
                break;
            case 8:
                mufasser.location = scene_finalArea(mufasser);
                break;
            case 99:
                typeText("\nGood ending! You successfully rescue Ayan from the Void Dimension and live happily.\n", 40);
                running = false;
                break;
            case -1:
                typeText("\n*** Bad ending. Ayan was killed and now your soul is trapped eternally. ***\n", 40);
                running = false;
                break;
            default:
                cout << "Unknown location. Exiting.\n";
                running = false;
        }

        if (mufasser.health > 100) mufasser.health = 100;

    }

    typeText("\nThank you for playing! \n", 40);
    cout << "\033[0m";
    return 0;
}

void displayStatus(const Player &p) {
    cout << "\n=== Status ===\n";
    cout << "Name: " << p.name << endl;
    cout << "Health: " << p.health << endl;
    cout << "Items (" << p.inv_count << "): ";
    if (p.inv_count == 0) cout << "None";
    else {
        for (int i = 0; i < p.inv_count; i++) {
            if (i) cout << ", ";
            cout << p.inventory[i];
        }
    }
    cout << "\n==============\n\n";
}
