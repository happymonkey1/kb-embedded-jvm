plugins {
    id("java")
}

group = "com.kablunk.KbJavaExample"
version = "1.0"

repositories {
    mavenCentral()
}

dependencies {
    // testImplementation(platform("org.junit:junit-bom:5.10.0"))
    // testImplementation("org.junit.jupiter:junit-jupiter")
}

tasks.test {
    useJUnitPlatform()
}

tasks.withType<Jar> {
    manifest {
        attributes["Main-Class"] = "com.kablunk.KbJavaExample.Main"
    }
}