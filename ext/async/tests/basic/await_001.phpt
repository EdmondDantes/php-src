--TEST--
Await function
--FILE--
<?php

Async\run(function() {
    echo "async function 1\n";

    echo Async\await(function() {
        echo "async function 2\n";
        return "result from async function 2\n";
    });
});

Async\run(function() {
    echo "async function 3\n";
});


echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
async function 1
async function 3
async function 2
result from async function 2
end
