import itertools
import json
import os
import re
import subprocess
import sys

diffs = ['Hard', 'VeryHard', 'CheatInsane']
races = ['terran', 'protoss', 'zerg']
map_names = ['map1', 'map2', 'map3']
matchups = list(itertools.product(diffs, races, map_names))

class Matchup:
    def __init__(self, difficulty, race, map_name):
        self.difficulty = difficulty
        self.race = race
        self.map_name = map_name

def main():
    if len(sys.argv) < 3:
        print('Invalid arguments')
        print('Usage:')
        print(sys.argv[0] + ' "<path/to/MulleMech.exe>" <matches-per-matchup>')
        return

    exe = sys.argv[1]
    matches = int(sys.argv[2])

    while True:
        run_game(exe, matches)

def run_game(exe, matches):
    results = {}
    try:
        with open('results.json', 'r') as fobj:
            results = json.load(fobj)
            verify_data(results) # will kill script with stacktrace
    except FileNotFoundError:
        results = setup_data()

    matchup = pick_matchup(matches, results)
    print("Playing versus " + matchup.difficulty + " " + matchup.race + " on " + matchup.map_name)
    output = subprocess.check_output([exe, '-c', '-a', matchup.race, '-d', matchup.difficulty, '-p', matchup.map_name])
    last_line = output[-256:].decode('utf-8').splitlines()[-1]
    second_last_line = output[-256:].decode('utf-8').splitlines()[-2]
    print('Game ended with last two lines:')
    print(second_last_line)
    print(last_line)
    match = re.search(r'Winning player id: \{(\d+)\}', last_line)
    if match:
        player_id = int(match.group(1))
        results[matchup.difficulty][matchup.race][matchup.map_name]['total'] += 1
        if player_id == 1:
            results[matchup.difficulty][matchup.race][matchup.map_name]['wins'] += 1
            print('We won!')
        else:
            print('They won :(')
        rename_replay(second_last_line, matchup)
    else:
        print('Match ended with no winner?!')

    with open('results.json', 'w') as fobj:
        json.dump(results, fobj, indent=4)

def verify_data(results):
    for matchup in matchups:
        int(results[matchup[0]][matchup[1]][matchup[2]]['wins'])
        int(results[matchup[0]][matchup[1]][matchup[2]]['total'])

def setup_data():
    res = {}
    for matchup in matchups:
        diff = matchup[0]
        race = matchup[1]
        map = matchup[2]
        if diff not in res:
            res[diff] = {}
        if race not in res[diff]:
            res[diff][race] = {}
        res[diff][race][map] = {'wins': 0, 'total': 0}
    return res

def pick_matchup(matches, results):
    for matchup in matchups:
        if results[matchup[0]][matchup[1]][matchup[2]]['total'] < matches:
            return Matchup(matchup[0], matchup[1], matchup[2])

def rename_replay(replay_line, matchup):
    res = re.search('Replay saved to (MulleMech_([0-9_]+).SC2Replay)', replay_line)
    if res:
        os.rename(res.group(1), matchup.difficulty + '_' + matchup.race + '_' + matchup.map_name.split('.')[0] + '__' + res.group(2) + '.SC2Replay')

if __name__ == '__main__':
    main()
