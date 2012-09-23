

from __future__ import division

from scale import Scale

chromatic_knotes = [
[(1,1), (3,1), None , (6,1), (8,1), (10,1), None , (1,2), (3,2)],
[(0,1), (2,1), (4,1), (5,1), (7,1), (9,1), (11,1), (0,2), (2,2), (4,2)],
[None , (1,0), (3,0), None , (6,0), (8,0), (10,0)],
[(0,0), (2,0), (4,0), (5,0), (7,0), (9,0), (11,0)]
]

chromatic_buttons = {
(0,  0): {'color': 'dark', 'enabled': True },
(0,  1): {'color': 'dark', 'enabled': True },
(0,  2): {'color': None  , 'enabled': False},
(0,  3): {'color': 'dark', 'enabled': True },
(0,  4): {'color': 'dark', 'enabled': True },
(0,  5): {'color': 'dark', 'enabled': True },
(0,  6): {'color': None  , 'enabled': False},
(0,  7): {'color': 'dark', 'enabled': True },
(0,  8): {'color': 'dark', 'enabled': True },
(0,  9): {'color': None  , 'enabled': False},
(0, 10): {'color': None  , 'enabled': True },

(1,  0): {'color': 'light', 'enabled': True },
(1,  1): {'color': 'light', 'enabled': True },
(1,  2): {'color': 'light', 'enabled': True },
(1,  3): {'color': 'light', 'enabled': True },
(1,  4): {'color': 'light', 'enabled': True },
(1,  5): {'color': 'light', 'enabled': True },
(1,  6): {'color': 'light', 'enabled': True },
(1,  7): {'color': 'light', 'enabled': True },
(1,  8): {'color': 'light', 'enabled': True },
(1,  9): {'color': 'light', 'enabled': True },
(1, 10): {'color': 'light', 'enabled': True },

(2,  0): {'color': None  , 'enabled': False},
(2,  1): {'color': 'dark', 'enabled': True },
(2,  2): {'color': 'dark', 'enabled': True },
(2,  3): {'color': None  , 'enabled': False},
(2,  4): {'color': 'dark', 'enabled': True },
(2,  5): {'color': 'dark', 'enabled': True },
(2,  6): {'color': 'dark', 'enabled': True },
(2,  7): {'color': None  , 'enabled': False},
(2,  8): {'color': 'dark', 'enabled': True },
(2,  9): {'color': 'dark', 'enabled': True },
(2, 10): {'color': None  , 'enabled': False},

(3,  0): {'color': 'light', 'enabled': True },
(3,  1): {'color': 'light', 'enabled': True },
(3,  2): {'color': 'light', 'enabled': True },
(3,  3): {'color': 'light', 'enabled': True },
(3,  4): {'color': 'light', 'enabled': True },
(3,  5): {'color': 'light', 'enabled': True },
(3,  6): {'color': 'light', 'enabled': True },
(3,  7): {'color': 'light', 'enabled': True },
(3,  8): {'color': 'light', 'enabled': True },
(3,  9): {'color': 'light', 'enabled': True },
(3, 10): {'color': 'light', 'enabled': True },
}


chromatic = Scale({
                  'ref_pitch': 440 * 2**(3/12),
                  'octave_ratio': ['/', [2, 1]],
                  'notes': list(zip(('C', 'C#', 'D', 'D#', 'E', 'F',
                                     'F#', 'G', 'G#', 'A', 'A#', 'B'),
                      (['c', cents] for cents in range(0, 1200, 100)))),
                  'knotes': chromatic_knotes,
                  'buttons': chromatic_buttons,
                  'name': 'chromatic'
                  })
slendro_intervals = [0,
             0 + 245,
             0 + 245 + 262,
             0 + 245 + 262 + 228,
             0 + 245 + 262 + 228 + 240]
             #0 + 245 + 262 + 228 + 240 + 230]

slendro_knotes = [
[(1,2), (3,2), None , (1,3), (3,3), None , (1,4), (3,4), None],
[(0,2), (2,2), (4,2), (0,3), (2,3), (4,3), (0,4), (2,4), (4,4), (0,5)],
[None , (1,0), (3,0), None , (1,1), (3,1), None],
[(0,0), (2,0), (4,0), (0,1), (2,1), (4,1), None]
]

slendro_buttons = {
(0,  0): {'color': 'light', 'enabled': True },
(0,  1): {'color': 'light', 'enabled': True },
(0,  2): {'color': None   , 'enabled': False},
(0,  3): {'color': 'light', 'enabled': True },
(0,  4): {'color': 'light', 'enabled': True },
(0,  5): {'color': None   , 'enabled': False},
(0,  6): {'color': 'light', 'enabled': True },
(0,  7): {'color': 'light', 'enabled': True },
(0,  8): {'color': None   , 'enabled': False},
(0,  9): {'color': 'light', 'enabled': True },
(0, 10): {'color': 'light', 'enabled': True },

(1,  0): {'color': 'light', 'enabled': True },
(1,  1): {'color': 'light', 'enabled': True },
(1,  2): {'color': 'light', 'enabled': True },
(1,  3): {'color': 'light', 'enabled': True },
(1,  4): {'color': 'light', 'enabled': True },
(1,  5): {'color': 'light', 'enabled': True },
(1,  6): {'color': 'light', 'enabled': True },
(1,  7): {'color': 'light', 'enabled': True },
(1,  8): {'color': 'light', 'enabled': True },
(1,  9): {'color': 'light', 'enabled': True },
(1, 10): {'color': 'light', 'enabled': True },

(2,  0): {'color': None   , 'enabled': False},
(2,  1): {'color': 'light', 'enabled': True },
(2,  2): {'color': 'light', 'enabled': True },
(2,  3): {'color': None   , 'enabled': False},
(2,  4): {'color': 'light', 'enabled': True },
(2,  5): {'color': 'light', 'enabled': True },
(2,  6): {'color': None   , 'enabled': False},

(3,  0): {'color': 'light', 'enabled': True },
(3,  1): {'color': 'light', 'enabled': True },
(3,  2): {'color': 'light', 'enabled': True },
(3,  3): {'color': 'light', 'enabled': True },
(3,  4): {'color': 'light', 'enabled': True },
(3,  5): {'color': 'light', 'enabled': True },
(3,  6): {'color': None   , 'enabled': False},
}
slendro = Scale({
                  'ref_pitch': 440 * 2**(3/12),
                  'octave_ratio': ['c', 1205],
                  'notes': list(zip(('ji', 'ro', 'lu', 'ma', 'nam'),
                      (['c', cents] for cents in slendro_intervals))),
                  'knotes': slendro_knotes,
                  'buttons': slendro_buttons,
                  'name': 'slendro'
                  })

