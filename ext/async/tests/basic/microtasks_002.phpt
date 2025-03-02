--TEST--
Async\defer should be executed before Async\async and Async\run
--FILE--
<?php

Async\defer(function() {
    echo "async function defer 1\n";

    Async\defer(function() {
        echo "async function defer 2\n";

        Async\defer(function() {
            echo "async function defer 3\n";
        });
    });
});

Async\async(function() {
    echo "async function 1\n";
});

Async\run(function() {
    echo "async function 2\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function defer 1
async function defer 2
async function defer 3
async function 1
async function 2
end
