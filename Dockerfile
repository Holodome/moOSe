FROM randomdude/gcc-cross-x86_64-elf AS builder

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y grub-common

COPY ./ /root/

WORKDIR /root/
RUN make moose.img

FROM scratch AS export
COPY --from=builder /root/moose.img .