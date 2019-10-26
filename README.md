# procgen
A place to play around with graphical procedural generation

## beat_square

This pattern appears visually random, but once in a while particles align to form visible square  boundaries. I am guessing this has something to do with integer truncation in velocity  initialization.

Lively discussion on reddit is here: https://old.reddit.com/r/proceduralgeneration/comments/dmyscn/patterns_emerging_from_an_accidentally_badly/

![beat_square pattern example](https://github.com/SemanticDevice/procgen/blob/master/beat_square.gif)

## beat_circle

Somewhat similar to the beat_square pattern, but velocities are deliberately chosen from a discrete range with values that are multiples of each other. Particles also wrap around at a circle boundary instead of a square.

![beat_circle pattern example](https://github.com/SemanticDevice/procgen/blob/master/beat_circle.gif)

## contrails

An attempt at making a visualization similar to this:

https://player.vimeo.com/video/341078895?autoplay=0&loop=0&autopause=0?muted=1&background=0

which was plucked from the front page of https://nannou.cc/.

Color pallet that looks interesting: https://coolors.co/export/png/5eb0c5-fff275-ff8c42-ff3c38-a23e48