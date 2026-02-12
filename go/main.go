package main

import (
	"log"
	"net"
	"os"
	"os/signal"
	"time"

	"github.com/cilium/ebpf/link"
)

func main() {
	var objs counterObjects
	if err := loadCounterObjects(&objs, nil); err != nil {
		log.Fatal("Load ebpf objects:", err)
	}
	defer objs.Close()

	ifname := "lan25g"
	iface, err := net.InterfaceByName(ifname)
	if err != nil {
		log.Fatalf("Get interface by name %s: %s", ifname, err)
	}

	link, err := link.AttachXDP(link.XDPOptions{
		Program:   objs.CountPackets,
		Interface: iface.Index,
	})
	defer link.Close()

	tick := time.Tick(time.Second)
	stop := make(chan os.Signal, 5)
	signal.Notify(stop, os.Interrupt)
	var last counterIngressT
	for {
		select {
		case <-tick:
			var count counterIngressT
			err := objs.PktCount.Lookup(uint32(0), &count)

			if err != nil {
				log.Fatal("Map lookup:", err)
			}
			log.Printf("Received %d pkt/s, %d byte/s", count.Packets-last.Packets, count.Bytes-last.Bytes)
			last = count
		case <-stop:
			log.Print("Received signal, exiting..")
			return
		}
	}
}
