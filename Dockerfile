FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
	&& apt-get install --no-install-recommends -y \
		build-essential \
		ca-certificates \
		cmake \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build \
		-DBUILD_TESTING=OFF \
		-DCMAKE_BUILD_TYPE=Release \
	&& cmake --build build --target puzzle_server --parallel

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
	&& apt-get install --no-install-recommends -y \
		ca-certificates \
		libstdc++6 \
	&& rm -rf /var/lib/apt/lists/* \
	&& useradd --create-home --uid 10001 appuser

WORKDIR /app

COPY --from=build /app/build/puzzle_server /app/puzzle_server

USER appuser

EXPOSE 8080

ENTRYPOINT ["/app/puzzle_server"]
