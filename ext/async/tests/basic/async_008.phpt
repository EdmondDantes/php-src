--TEST--
Async without scheduler call
--FILE--
<?php

Async\async(function() {
    echo "async function 1\n";

	Async\async(function() {
		echo "async function 1-1\n";

		Async\async(function() {
			echo "async function 1-1-1\n";
		});
	});
});

Async\run(function() {
    echo "async function 2\n";
});

Async\async(function() {
    echo "async function 3\n";

	Async\async(function() {
		echo "async function 3-1\n";

		Async\async(function() {
			echo "async function 3-1-1\n";
		});
	});

});


echo "start\n";
echo "end\n";

?>
--EXPECT--
start
end
