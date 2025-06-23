import subprocess

# Run the program, capturing both stdout and stderr
result = subprocess.run(
    ["./test.exe"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True          # get strings instead of bytes
)

# Always check the return code
if result.returncode != 0:
    print(f"Program failed (code={result.returncode})")
    print("Standard error output:")
    print(result.stderr)
else:
    print("Program succeeded. Output was:")
    print(result.stdout)
