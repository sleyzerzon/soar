#!/bin/bash

if [ "$CORES" == "" ]; then
  CORES=1
fi

AGGREGATE=8
EPISODES=10000

declare -a VALS=( \
                 10 10
                 10 20
                 20 10
                 20 20
                 )
declare -a VALSP=( \
                  -1 0 0
                  -1 0 0
                  -1 0 0
                  -1 0 0
                 )

random=($(cat experiment/random))

if [ ${#random[@]} -eq 0 ]; then
  for i in $(seq 0 $(($AGGREGATE - 1))); do
    random[$i]=$RANDOM
  done

  echo ${random[@]} > experiment/random
fi

for v in $(seq 0 $((${#VALS[@]} / 2 - 1))); do
  BASE="experiment"
  START="${VALS[$((2 * v + 0))]}-${VALS[$((2 * v + 1))]}"
  SP="${VALSP[$((3 * v + 0))]}_${VALSP[$((3 * v + 1))]}-${VALSP[$((3 * v + 2))]}"
  DIR="$BASE/${START}_$SP"
  mkdir -p $DIR
  rm -i $DIR/*
  DIRS[v]=$DIR
done

experiment () {
  echo time out/PuddleWorld --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar --sp-special ${VALSP[$((3 * $3 + 0))]} ${VALSP[$((3 * $3 + 1))]} ${VALSP[$((3 * $3 + 2))]} \> $2/puddleworld-$1.out
       time out/PuddleWorld --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar --sp-special ${VALSP[$((3 * $3 + 0))]} ${VALSP[$((3 * $3 + 1))]} ${VALSP[$((3 * $3 + 2))]}  > $2/puddleworld-$1.out
  echo ""
}

experiment_sub () {
  for v in $(seq $1 $2 $((${#random[@]} - 1))); do
    experiment ${random[v]} $3 $4
  done
}

experiments () {
  for v in $(seq $1 $2 $((${#VALS[@]} / 2 - 1))); do
    DIR=${DIRS[v]}

#     echo $DIR
#     continue

    cp PuddleWorld/puddle-world.soar $DIR/in.soar
    echo "sp {apply*initialize*puddleworld
              (state <s> ^operator.name puddleworld)
          -->
              (<s> ^name puddleworld
                  ^div <d>)
              (<d> ^name default
                  ^x (/ 1.001 ${VALS[$((2 * v + 0))]})
                  ^y (/ 1.001 ${VALS[$((2 * v + 1))]}))
          }" >> $DIR/in.soar

#     for r in ${random[@]}; do
#       experiment $r $DIR $v
#     done

    for x in $(seq 0 $(($CORES - 1))); do
      experiment_sub $x $2 $DIR $v &
    done
    wait 

    ./puddleworld.py $DIR/*.out
  done
}

for x in $(seq 0 $(($CORES - 1))); do
#   echo experiments $((4 * x)) $((4 * $CORES))
  experiments $x $CORES #&
done
wait
