<?php

echo "hello";
usleep(1000);
fwrite(STDERR, "SOME ERROR");
usleep(1000);
echo "world";
