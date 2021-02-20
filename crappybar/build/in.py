def run():
    maxi = 6

    from datetime import datetime
    import subprocess
    subprocess = subprocess.Popen("i3-msg -t get_workspaces \
                                        | jq '.[] | select(.focused==true).name' \
                                        | cut -d'\"' -f2", shell=True, stdout=subprocess.PIPE)
    subprocess_return = subprocess.stdout.read()
    workspace = int(subprocess_return.decode("utf-8")[:-1])

    stri = "|"

    symbols = ['', '', 'ﭮ', '', '醙', '']

    for i in range(1,maxi+1):
        symbol = symbols[i-1]
        if i == workspace:
            stri += "{}|".format(symbol)
        else:
            stri += "{}|".format(i)

    print(stri, end="")

    now = datetime.now()

    current_time = now.strftime("%d.%m.%Y  %H:%M:%S")

    print(' '*(1920//16+16*2+4)+current_time)

while True:
    run()
