package main

import (
	"io"
	"log"
	"net"
	"os"
	"os/exec"
)

func main() {
	if len(os.Args) == 1 || os.Args[1] == "-h" || os.Args[1] == "--help" || len(os.Args) != 3 {
		println("Usage: flowsnd IP:PORT RATE")
		os.Exit(0)
	}

	remote, err := net.Dial("tcp", os.Args[1])
	if err != nil {
		log.Fatalf("Connect to %s: %s", os.Args[1], err.Error())
	}

	pv := exec.Command("pv", "-L", os.Args[2], "/dev/zero")
	pv.Stderr = os.Stderr
	stdout, err := pv.StdoutPipe()
	if err != nil {
		log.Fatalf("Can't run pv: %s", err.Error())
	}

	go io.Copy(remote, stdout)
	pv.Start()
	pv.Wait()
}
