import java.util.Base64

plugins {
    id("com.android.application")
}

val ciVersionCode = System.getenv("GITHUB_RUN_NUMBER")?.toIntOrNull()?.coerceAtLeast(2) ?: 2
val testKeyBase64File = rootProject.file("pokeemerald-test.keystore.b64")
val testKeyFile = rootProject.file("build/pokeemerald-test.keystore")
testKeyFile.parentFile.mkdirs()
testKeyFile.writeBytes(Base64.getMimeDecoder().decode(testKeyBase64File.readText()))

android {
    namespace = "com.thabombs.pokeemeraldnative"
    compileSdk = 35
    ndkVersion = "27.2.12479018"

    signingConfigs {
        create("testDebug") {
            // Public, development-only key. Keeping it stable lets test APKs
            // update in place across ephemeral GitHub Actions runners.
            storeFile = testKeyFile
            storePassword = "android"
            keyAlias = "pokeemerald-test"
            keyPassword = "android"
        }
    }

    defaultConfig {
        applicationId = "com.thabombs.pokeemeraldnative"
        minSdk = 26
        targetSdk = 35
        versionCode = ciVersionCode
        versionName = "0.2.$ciVersionCode"

        externalNativeBuild {
            cmake {
                arguments += listOf("-DANDROID_STL=none")
            }
        }

        ndk {
            abiFilters += listOf("arm64-v8a")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        debug {
            isJniDebuggable = true
            signingConfig = signingConfigs.getByName("testDebug")
        }
        release {
            isMinifyEnabled = false
        }
    }
}
