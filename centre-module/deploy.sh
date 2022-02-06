echo Deploying Centre Module to the Pico.
script="$0"
basename="${script%/*}"

openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ${basename}/../build/centre-module/centre_module.elf verify reset exit"

